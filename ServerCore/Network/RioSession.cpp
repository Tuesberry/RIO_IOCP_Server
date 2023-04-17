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
	, m_rioCore(weak_ptr<RioCore>())
	, m_recvEvent()
	, m_sendEvent()
	, m_requestQueue()
	, m_recvBufId()
	, m_sendBufId()
	, m_recvBuffer(nullptr)
	, m_sendBuffer(nullptr)
{
}

/* --------------------------------------------------------
*	Method:		RioSession::Connect
*	Summary:	connect
-------------------------------------------------------- */
bool RioSession::Connect()
{
	// TODO : connect
	return true;
}

/* --------------------------------------------------------
*	Method:		RioSession::Disconnect
*	Summary:	disconnect session
-------------------------------------------------------- */
void RioSession::Disconnect()
{
	// close socket
	closesocket(m_socket);

	// on disconnected(redefined in application code)
	OnDisconnected();

	// release session from RioCore
	m_rioCore.lock()->ReleaseSession(static_pointer_cast<RioSession>(shared_from_this()));
}

/* --------------------------------------------------------
*	Method:		RioSession::Send
*	Summary:	send packet 
-------------------------------------------------------- */
void RioSession::Send(char* buf, int len)
{
	lock_guard<mutex> lock(m_lock);

	char* writeBuf = m_sendBuffer->GetWriteBuf();
	memcpy(writeBuf, buf, len);
	m_sendBuffer->OnWriteBuffer(len);

	RegisterSend();
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
		ProcessSend(bytesTransferred);
		break;
	default:
		break;
	}
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
	m_recvEvent.Length = m_recvBuffer->GetFreeSize();	//TODO
	m_recvEvent.Offset = m_recvBuffer->GetWritePos(); //TODO

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
void RioSession::RegisterSend()
{
	if (IsConnected() == false)
		return;

	m_sendEvent.m_owner = shared_from_this();

	m_sendEvent.BufferId = m_sendBufId;
	m_sendEvent.Length = m_sendBuffer->GetDataSize(); //TODO
	m_sendEvent.Offset = m_sendBuffer->GetReadPos(); //TODO

	DWORD bytesTransferred = 0;
	DWORD flags = 0;

	if (SocketCore::RIO.RIOSend(m_requestQueue, (PRIO_BUF)&m_sendEvent, SEND_BUFF_COUNT, flags, &m_sendEvent) == false)
	{
		m_sendEvent.m_owner = nullptr;
		Disconnect();
	}
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
	int processLen = OnRecv(m_recvBuffer->GetReadBuf(), bytesTransferred);
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

void RioSession::ProcessSend(int bytesTransferred)
{
	m_sendEvent.m_owner = nullptr;

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
	OnRecvPacket(buffer, len);
	return len;
	/*
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
	*/
}

bool RioSession::InitSession()
{
	if (AllocBuffer() == false)
		return false;

	if (CreateRequestQueue() == false)
		return false;

	return true;
}

bool RioSession::AllocBuffer()
{
	m_recvBuffer = make_shared<RioBuffer>(BUFFER_SIZE);
	m_sendBuffer = make_shared<RioBuffer>(BUFFER_SIZE);

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

	m_requestQueue = SocketCore::RIO.RIOCreateRequestQueue(
		m_socket, 
		MAX_SEND_RQ_SIZE,
		SEND_BUFF_COUNT,
		MAX_RECV_RQ_SIZE,
		RECV_BUFF_COUNT,
		rioCore->GetCompletionQueue(),
		rioCore->GetCompletionQueue(),
		NULL
	);
	if (m_requestQueue == RIO_INVALID_RQ)
	{
		HandleError("RIOCreateRequestQueue");
		return false;
	}

	return true;
}
