#include "RioSession.h"
#include "SocketCore.h"
#include "RioCore.h"
#include "RioBuffer.h"	

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

bool RioSession::Connect()
{
	return true;
}

void RioSession::Disconnect()
{
	closesocket(m_socket);

	OnDisconnected();
	m_rioCore.lock()->ReleaseSession(static_pointer_cast<RioSession>(shared_from_this()));
}

void RioSession::Send()
{

}

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

void RioSession::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	m_recvEvent.m_owner = shared_from_this();
	
	m_recvEvent.BufferId = m_recvBufId;
	m_recvEvent.Length = m_recvBuffer->GetBufferSize();	//TODO
	m_recvEvent.Offset = 0; //TODO

	DWORD recvbytes = 0;
	DWORD flags = 0;

	if (SocketCore::RIO.RIOReceive(m_requestQueue, (PRIO_BUF)&m_recvEvent, RECV_BUFF_COUNT, flags, &m_recvEvent) == false)
	{
		m_recvEvent.m_owner = nullptr;
		HandleError("RioReceive");
		Disconnect();
	}
}

void RioSession::RegisterSend()
{
	if (IsConnected() == false)
		return;

	m_sendEvent.m_owner = shared_from_this();

	m_sendEvent.BufferId = m_sendBufId;
	m_sendEvent.Length = 0; //TODO
	m_sendEvent.Offset = 0; //TODO

	DWORD bytesTransferred = 0;
	DWORD flags = 0;

	if (SocketCore::RIO.RIOSend(m_requestQueue, (PRIO_BUF)&m_sendEvent, SEND_BUFF_COUNT, flags, &m_sendEvent) == false)
	{
		m_sendEvent.m_owner = nullptr;
		Disconnect();
	}
}

void RioSession::ProcessConnect()
{
	m_bConnected.store(true);

	// make socket non-blocking
	u_long arg = 1;
	ioctlsocket(m_socket, FIONBIO, &arg);

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

	int processLen = OnRecv(m_recvBuffer->GetBuffer(), bytesTransferred);
	memset(m_recvBuffer->GetBuffer(), 0, m_recvBuffer->GetBufferSize());

	RegisterRecv();
}

void RioSession::ProcessSend(int bytesTransferred)
{
	m_sendEvent.m_owner = nullptr;

	OnSend(bytesTransferred);
	memset(m_sendBuffer->GetBuffer(), 0, m_sendBuffer->GetBufferSize());
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
