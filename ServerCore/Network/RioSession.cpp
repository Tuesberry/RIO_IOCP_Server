#include "RioSession.h"
#include "SocketCore.h"
#include "RioCore.h"
#include "RioBuffer.h"	

/* --------------------------------------------------------
*	Method:		RioSession::RioSession
*	Summary:	Constructor
-------------------------------------------------------- */
RioSession::RioSession()
	: m_socket(INVALID_SOCKET)
	, m_address()
	, m_bConnected(false)	
	, m_bAllocated(false)
	, m_rioCore(weak_ptr<RioCore>())
	, m_recvEvent()
	, m_sendBufQueue()
	, m_sendQueueLock()
	, m_sendCnt(0)
	, m_lastSendTime(0)
	, m_requestQueue()
	, m_recvBufId(NULL)
	, m_sendBufId(NULL)
	, m_recvBuffer(nullptr)
	, m_sendBuffer(nullptr)
{
}

/* --------------------------------------------------------
*	Method:		RioSession::!RioSession
*	Summary:	Destructor
-------------------------------------------------------- */
RioSession::~RioSession()
{
	SocketCore::RIO.RIODeregisterBuffer(m_recvBufId);
	SocketCore::RIO.RIODeregisterBuffer(m_sendBufId);
}

/* --------------------------------------------------------
*	Method:		RioSession::Disconnect
*	Summary:	disconnect session
-------------------------------------------------------- */
void RioSession::Disconnect()
{
	if (m_bConnected.exchange(false) == false)
	{
		return;
	}

	// no TCP TIME_WAIT
	if (!SocketCore::SetLinger(m_socket, true, 0))
	{
		HandleError("SetLinger");
		return;
	}
	
	// close socket
	SocketCore::Close(m_socket);

	// on disconnected(redefined in application code)
	OnDisconnected();

	// release session from RioCore
	m_rioCore.lock()->ReleaseSession(shared_from_this());
}

/* --------------------------------------------------------
*	Method:		RioSession::Send
*	Summary:	save sendPacket in queue
-------------------------------------------------------- */
void RioSession::Send(shared_ptr<SendBuffer> sendBuffer)
{
	if (IsConnected() == false)
		return;

	{
		lock_guard<mutex> lock(m_sendQueueLock);
		m_sendBufQueue.push(sendBuffer);
	}
}

/* --------------------------------------------------------
*	Method:		RioSession::Dispatch
*	Summary:	process rioEvent
-------------------------------------------------------- */
void RioSession::Dispatch(RioEvent* rioEvent, int bytesTransferred)
{
	switch (rioEvent->m_ioType)
	{
	case RIO_IO_TYPE::RECV:
		ProcessRecv(bytesTransferred);
		break;
	case RIO_IO_TYPE::SEND:
		ProcessSend(bytesTransferred, static_cast<RioSendEvent*>(rioEvent));
		break;
	default:
		break;
	}
}

/* --------------------------------------------------------
*	Method:		RioSession::SendDeferred
*	Summary:	deferred send
-------------------------------------------------------- */
bool RioSession::SendDeferred()
{
	if (IsConnected() == false || IsAllocated() == false)
		return false;

	// time since last commit > 20 => continue
	long int currentTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	if (currentTime - m_lastSendTime <= COMMIT_TIME)
	{
		return false;
	}

	shared_ptr<SendBuffer> sendBuffer;
	int sendCount = 0;

	while (true)
	{
		// dequeue msg from queue
		{
			lock_guard<mutex> lock(m_sendQueueLock);
			
			if (m_sendBufQueue.empty())
			{
				break;
			}
			
			sendBuffer = m_sendBufQueue.front();
			
			if (!m_sendBuffer->WriteBuffer((char*)sendBuffer->GetData(), sendBuffer->GetDataSize()))
			{
				break;
			}

			m_sendBufQueue.pop();
		}

		// deferred Send
		while (m_sendBuffer->GetSendDataSize() > 0)
		{
			if (!RegisterSend(m_sendBuffer->GetChunkSendSize(), m_sendBuffer->GetSendOffset()))
			{
				break;
			}

			sendCount++;
		}

		// check sendCount > MAX_POST_CNT
		if (sendCount >= MAX_POST_CNT)
		{
			cout << "EXCEED MAX_POST_CNT" << endl;
			break;
		}
	}

	m_lastSendTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();

	// rio send commit
	if (sendCount > 0)
	{
		SendCommit();
	}
	
	return true;
}

bool RioSession::SendDeferredSG()
{
	if (IsConnected() == false || IsAllocated() == false)
		return false;

	// time since last commit > 20 => continue
	long int currentTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	if (currentTime - m_lastSendTime <= COMMIT_TIME)
	{
		return false;
	}

	shared_ptr<SendBuffer> sendBuffer;
	int sendCount = 0;

	while (true)
	{
		// dequeue msg from queue
		{
			lock_guard<mutex> lock(m_sendQueueLock);

			if (m_sendBufQueue.empty())
			{
				break;
			}

			sendBuffer = m_sendBufQueue.front();

			if (!m_sendBuffer->WriteBuffer((char*)sendBuffer->GetData(), sendBuffer->GetDataSize()))
			{
				break;
			}

			m_sendBufQueue.pop();
		}

		if (m_sendBuffer->GetSendDataSize() < SEND_LIMIT)
		{
			continue;
		}

		// deferred Send
		while (m_sendBuffer->GetSendDataSize() > 0)
		{
			if (!RegisterSend(m_sendBuffer->GetChunkSendSize(), m_sendBuffer->GetSendOffset()))
			{
				break;
			}

			sendCount++;
		}

		// check sendCount > MAX_POST_CNT
		if (sendCount > MAX_POST_CNT)
		{
			break;
		}
	}

	if (sendCount < MAX_POST_CNT)
	{
		while (m_sendBuffer->GetSendDataSize() > 0)
		{
			if (!RegisterSend(m_sendBuffer->GetChunkSendSize(), m_sendBuffer->GetSendOffset()))
			{
				break;
			}

			sendCount++;

			if (sendCount > MAX_POST_CNT)
			{
				break;
			}
		}
	}

	m_lastSendTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();

	// rio send commit
	if (sendCount > 0)
	{
		SendCommit();
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		RioSession::SendCommit
*	Summary:	send commit
-------------------------------------------------------- */
void RioSession::SendCommit()
{
	if (m_sendCnt.load() <= 0)
		return;

	if (SocketCore::RIO.RIOSend(m_requestQueue, NULL, 0, RIO_MSG_COMMIT_ONLY, NULL) == false)
	{
		HandleError("SendCommit");
		Disconnect();
	}

	m_sendCnt.store(0);
}

/* --------------------------------------------------------
*	Method:		RioSession::RegisterRecv
*	Summary:	RIOReceive
-------------------------------------------------------- */
void RioSession::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	m_recvEvent.m_owner = shared_from_this();

	m_recvEvent.BufferId = m_recvBufId;
	m_recvEvent.Length = m_recvBuffer->GetFreeSize();	
	m_recvEvent.Offset = m_recvBuffer->GetWritePos(); 

	DWORD recvbytes = 0;
	DWORD flags = 0;

	if (SocketCore::RIO.RIOReceive(m_requestQueue, (PRIO_BUF)&m_recvEvent, RECV_BUFF_COUNT, flags, &m_recvEvent) == false)
	{
		m_recvEvent.m_owner = nullptr;

		HandleError("RioReceive");
		Disconnect();
	}
}

/* --------------------------------------------------------
*	Method:		RioSession::RegisterSend
*	Summary:	RIOSend
-------------------------------------------------------- */
bool RioSession::RegisterSend(int dataLength, int dataOffset)
{
	if (IsConnected() == false )
		return false;

	RioSendEvent* sendEvent = new RioSendEvent();

	sendEvent->m_owner = shared_from_this();
	sendEvent->BufferId = m_sendBufId;
	sendEvent->Length = dataLength;
	sendEvent->Offset = dataOffset;

	DWORD bytesTransferred = 0;
	DWORD flags = 0;

	if (SocketCore::RIO.RIOSend(m_requestQueue, (PRIO_BUF)sendEvent, SEND_BUFF_COUNT, RIO_MSG_DEFER, sendEvent) == false)
	{
		sendEvent->m_owner = nullptr;
		delete(sendEvent);

		int errCode = ::WSAGetLastError();
		if (errCode != WSAENOBUFS)
		{
			HandleError("RioSend");
			Disconnect();
		}
		else
		{
			cout << ThreadId << " | WSAENOBUFS, SendCount = " << m_sendCnt.load() << endl;
		}

		return false;
	}

	m_sendCnt.fetch_add(1);

	if (m_sendBuffer->OnSendBuffer(dataLength) == false)
	{
		HandleError("OnSendBufferError");
		Disconnect();

		return false;
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		RioSession::ProcessConnect
*	Summary:	process connect
-------------------------------------------------------- */
void RioSession::ProcessConnect()
{
	m_bConnected.store(true);

	// make socket non-blocking
	//u_long arg = 1;
	//ioctlsocket(m_socket, FIONBIO, &arg);

	// init session
	InitSession();

	// on connected
	OnConnected();

	// wait recv
	RegisterRecv();
}

void RioSession::ProcessRecv(int bytesTransferred)
{
	m_recvEvent.m_owner = nullptr;

	if (bytesTransferred == 0)
	{
		HandleError("bytesTransferred == 0");
		Disconnect();
		return;
	}

	if (m_recvBuffer->OnWriteBuffer(bytesTransferred) == false)
	{
		HandleError("Buffer Overflow");
		Disconnect();
		return;
	}

	int dataSize = m_recvBuffer->GetDataSize();
	int processLen = OnRecv(m_recvBuffer->GetReadBuf(), dataSize);
	if (processLen < 0 || dataSize < processLen || m_recvBuffer->OnReadBuffer(processLen) == false)
	{
		HandleError("RecvBuffer Read Error");
		Disconnect();
		return;
	}
	
	// Adjust
	m_recvBuffer->AdjustPos();

	// register recv
	RegisterRecv();
}

void RioSession::ProcessSend(int bytesTransferred, RioSendEvent* sendEvent)
{
	sendEvent->m_owner = nullptr;
	delete(sendEvent);
	
	// bytesTransferred 0 check
	if (bytesTransferred == 0)
	{
		HandleError("bytesTransferred == 0");
		Disconnect();
		return;
	}

	OnSend(bytesTransferred);

	// buffer 초기화
	if (m_sendBuffer->OnReadBuffer(bytesTransferred) == false)
	{
		HandleError("OnReadBuffer Error");
		Disconnect();
		return;
	}
}

int RioSession::OnRecv(char* buffer, int len)
{
	int processLen = 0;

	while (true)
	{
		// 전체 처리한 양이 받은 패킷 길이를 넘는가?
		if (processLen >= len)
			break;

		// 패킷 헤더 추출 가능?
		if (len - processLen < (int)sizeof(PacketHeader))
			break;

		// 패킷 사이즈 추출
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		if (len - processLen < header.size)
			break;

		// OnRecvPacket
		OnRecvPacket(&buffer[processLen], header.size);

		// adjust processLen
		processLen += header.size;
	}

	return processLen;
}

bool RioSession::InitSession()
{
	if (AllocBuffer() == false)
		return false;

	if (CreateRequestQueue() == false)
		return false;

	m_bAllocated.store(true);

	return true;
}

bool RioSession::AllocBuffer()
{
	m_recvBuffer = make_shared<RioBuffer>(BUFFER_SIZE);
	m_sendBuffer = make_shared<RioSendBuffer>(BUFFER_SIZE);

	m_recvBufId = SocketCore::RIO.RIORegisterBuffer(m_recvBuffer->GetBuffer(), BUFFER_SIZE);
	if (m_recvBufId == RIO_INVALID_BUFFERID)
	{
		HandleError("RioRegisterBuffer");
		return false;
	}
	
	m_sendBufId = SocketCore::RIO.RIORegisterBuffer(m_sendBuffer->GetBuffer(), BUFFER_SIZE);
	if (m_sendBufId == RIO_INVALID_BUFFERID)
	{
		HandleError("RioRegisterBuffer");
		return false;
	}

	return true;
}

bool RioSession::CreateRequestQueue()
{
	shared_ptr<RioCore> rioCore = m_rioCore.lock();
	
	if (rioCore == nullptr)
		return false;

#if SEPCQ
	m_requestQueue = SocketCore::RIO.RIOCreateRequestQueue(
		m_socket,
		MAX_RECV_RQ_SIZE,
		RECV_BUFF_COUNT,
		MAX_SEND_RQ_SIZE,
		SEND_BUFF_COUNT,
		rioCore->GetRecvCompletionQueue(),
		rioCore->GetSendCompletionQueue(),
		NULL
	);
#else
	m_requestQueue = SocketCore::RIO.RIOCreateRequestQueue(
		m_socket, 
		MAX_RECV_RQ_SIZE,
		RECV_BUFF_COUNT,
		MAX_SEND_RQ_SIZE,
		SEND_BUFF_COUNT,
		rioCore->GetCompletionQueue(),
		rioCore->GetCompletionQueue(),
		NULL
	);
#endif
	if (m_requestQueue == RIO_INVALID_RQ)
	{
		HandleError("RIOCreateRequestQueue");
		return false;
	}

	return true;
}
