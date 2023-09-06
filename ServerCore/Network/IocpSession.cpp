#include "IocpSession.h"
#include "SocketCore.h"
#include "IocpService.h"
#include "PacketHeader.h"

/* --------------------------------------------------------
*	Method:		IocpSession::IocpSession
*	Summary:	constructor
-------------------------------------------------------- */
IocpSession::IocpSession()
	: m_socket(INVALID_SOCKET)
	, m_address()
	, m_bConnected(false)
	, m_recvEvent()
	, m_connectEvent()
	, m_disconnectEvent()
	, m_sendEvent()
	, m_iocpService(nullptr)
	, m_recvBuffer(BUFSIZE)
	, m_sendBufferQueue()
	, m_sendBufLock()
	, m_bSendRegistered(false)
{
	m_socket = SocketCore::Socket();
	if (m_socket == INVALID_SOCKET)
	{
		HandleError("IocpSession::IocpSession");
	}
}

/* --------------------------------------------------------
*	Method:		IocpSession::~IocpSession
*	Summary:	destructor
-------------------------------------------------------- */
IocpSession::~IocpSession()
{
	SocketCore::Close(m_socket);
}

/* --------------------------------------------------------
*	Method:		IocpSession::Connect
*	Summary:	call RegisterConnect and return result
-------------------------------------------------------- */
bool IocpSession::Connect()
{
	return RegisterConnect();
}

/* --------------------------------------------------------
*	Method:		IocpSession::Disconnect
*	Summary:	if m_bConnected is true, 
*				RegisterDisconnect is called
-------------------------------------------------------- */
void IocpSession::Disconnect()
{
	if (m_bConnected.exchange(false) == false)
	{
		return;
	}

	RegisterDisconnect();
}

/* --------------------------------------------------------
*	Method:		IocpSession::Disconnect
*	Summary:	if m_bConnected is true,
*				RegisterDisconnect is called
-------------------------------------------------------- */
void IocpSession::Send(shared_ptr<NetBuffer> sendBuffer)
{
	if (!IsConnected())
	{
		return;
	}

	bool registered = false;
	{
		m_sendBufLock.lock();

		// enqueue sendbuffer
		m_sendBufferQueue.push(sendBuffer);

		// registerSend를 수행하고 있는 스레드가 없으면, reigster에 true를 할당
		if (m_bSendRegistered.exchange(true) == false)
		{
			registered = true;
		}

		m_sendBufLock.unlock();
	}

	if (registered)
	{
		RegisterSend();
	}
}

/* --------------------------------------------------------
*	Method:		IocpSession::GetHandle
*	Summary:	return m_socket
-------------------------------------------------------- */
HANDLE IocpSession::GetHandle()
{
	return reinterpret_cast<HANDLE>(m_socket);
}

/* --------------------------------------------------------
*	Method:		IocpSession::Dispatch
*	Summary:	Check the type of I/O and call the Process
*				function appropriate for the type
*	Args:		IocpEvent* iocpEvent
*					iocpEvent to process
*				int bytesTransferred
*					number of bytes sent or received
-------------------------------------------------------- */
void IocpSession::Dispatch(IocpEvent* iocpEvent, int bytesTransferred)
{
	IOCP_IO_TYPE ioType = iocpEvent->m_eventType;
	switch (ioType)
	{
	case IOCP_IO_TYPE::CONNECT:
		ProcessConnect();
		break;
	case IOCP_IO_TYPE::DISCONNECT:
		ProcessDisconnect();
		break;
	case IOCP_IO_TYPE::RECV:
		ProcessRecv(bytesTransferred);
		break;
	case IOCP_IO_TYPE::SEND:
		ProcessSend(bytesTransferred);
		break;
	default:
		HandleError("IocpSession::Dispatch");
		break;
	}
}

/* --------------------------------------------------------
*	Method:		IocpSession::RegisterConnect
*	Summary:	register connect request
-------------------------------------------------------- */
bool IocpSession::RegisterConnect()
{
	if (IsConnected())
	{
		return false;
	}

	if (m_iocpService->GetServiceType() != ServiceType::CLIENT)
	{
		return false;
	}

	// set reuse address
	if (!SocketCore::SetReuseAddr(m_socket, true))
	{
		return false;
	}

	// bind any address
	if (!SocketCore::BindAddrAny(m_socket, 0))
	{
		return false;
	}

	// init connect event
	m_connectEvent.Init();
	m_connectEvent.m_owner = shared_from_this();

	// conenctEx
	DWORD bytesTransferred = 0;
	SOCKADDR_IN sockAddr = m_iocpService->GetAddress().GetSockAddr();
	if (!SocketCore::ConnectEx(m_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &bytesTransferred, &m_connectEvent))
	{
		if (::WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_connectEvent.m_owner = nullptr;
			HandleError("ConnectEx");
			return false;
		}
	}
	
	return true;
}

/* --------------------------------------------------------
*	Method:		IocpSession::RegisterDisconnect
*	Summary:	register disconnect request
-------------------------------------------------------- */
void IocpSession::RegisterDisconnect()
{
	m_disconnectEvent.Init();
	m_disconnectEvent.m_owner = shared_from_this();

	if (!SocketCore::DisconnectEx(m_socket, &m_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		if (::WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_disconnectEvent.m_owner = nullptr;
			HandleError("DisconnectEx");
			return;
		}
	}
}

/* --------------------------------------------------------
*	Method:		IocpSession::RegisterRecv
*	Summary:	register recv request
-------------------------------------------------------- */
void IocpSession::RegisterRecv()
{
	if (!IsConnected())
	{
		return;
	}

	// init recvEvent
	m_recvEvent.Init();
	m_recvEvent.m_owner = shared_from_this();

	// set wsaBuf
	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<CHAR*>(m_recvBuffer.GetWriteBuf());
	wsaBuf.len = m_recvBuffer.GetChunkWriteSize();

	DWORD recvLen = 0;
	DWORD flags = 0;
	if (::WSARecv(m_socket, &wsaBuf, 1, &recvLen, &flags, &m_recvEvent, nullptr) == SOCKET_ERROR)
	{
		if (::WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_recvEvent.m_owner = nullptr;
			HandleError("WSARecv");
			return;
		}
	}
}

/* --------------------------------------------------------
*	Method:		IocpSession::RegisterSend
*	Summary:	register send request
-------------------------------------------------------- */
void IocpSession::RegisterSend()
{
	if (!IsConnected())
	{
		return;
	}

	// Scatter-Gather
	// init m_sendEvent
	m_sendEvent.Init();
	m_sendEvent.m_owner = shared_from_this();
	
	// add sendBuffer in sendEvent
	{
		m_sendBufLock.lock();
		while (m_sendBufferQueue.empty() == false)
		{
			shared_ptr<NetBuffer> sendBuf = m_sendBufferQueue.front();
			m_sendBufferQueue.pop();

			m_sendEvent.m_sendBuffer.push_back(sendBuf);
		}
		m_sendBufLock.unlock();
	}

	// add all sendData in WSABuf
	vector<WSABUF> wsaBuf;
	wsaBuf.reserve(m_sendEvent.m_sendBuffer.size());
	for (shared_ptr<NetBuffer> sendBuf : m_sendEvent.m_sendBuffer)
	{
		WSABUF buf;
		buf.buf = sendBuf->GetReadBuf();
		buf.len = sendBuf->GetDataSize();
		wsaBuf.push_back(buf);
	}
	
	// WSASend
	DWORD bytesTransferred = 0;
	if (::WSASend(m_socket, wsaBuf.data(), static_cast<DWORD>(wsaBuf.size()), &bytesTransferred, 0, &m_sendEvent, NULL) == SOCKET_ERROR)
	{
		if (::WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_sendEvent.m_owner = nullptr;
			m_sendEvent.m_sendBuffer.clear();

			m_bSendRegistered.store(false);

			HandleError("WSASend");
			
			return;
		}
	}

}

/* --------------------------------------------------------
*	Method:		IocpSession::ProcessConnect
*	Summary:	process completion of connect request
-------------------------------------------------------- */
void IocpSession::ProcessConnect()
{
	m_connectEvent.m_owner = nullptr;
	
	m_bConnected.store(true);

	// add session
	m_iocpService->AddSession(static_pointer_cast<IocpSession>(shared_from_this()));

	// on connected
	OnConnected();

	// Wait Recv
	RegisterRecv();
}

/* --------------------------------------------------------
*	Method:		IocpSession::ProcessDisconnect
*	Summary:	process completion of disconnect request
-------------------------------------------------------- */
void IocpSession::ProcessDisconnect()
{
	m_disconnectEvent.m_owner = nullptr;

	// on disconnected
	OnDisconnected();

	// release session from iocpService
	m_iocpService->ReleaseSession(static_pointer_cast<IocpSession>(shared_from_this()));
}

/* --------------------------------------------------------
*	Method:		IocpSession::ProcessRecv
*	Summary:	process completion of recv request
*	Args:		int bytesTransferred
*					number of bytes received 
-------------------------------------------------------- */
void IocpSession::ProcessRecv(int bytesTransferred)
{
	m_recvEvent.m_owner = nullptr;

	if (bytesTransferred == 0)
	{
		HandleError("ProcessRecv::ByteTransferred Error");
		Disconnect();
		return;
	}

	if (m_recvBuffer.OnWriteBuffer(bytesTransferred) == false)
	{
		HandleError("ProcessRecv::OnWrite Error");
		Disconnect();
		return;
	}

	// on recv
	int dataSize = m_recvBuffer.GetDataSize();
	int processLen = OnRecv(m_recvBuffer.GetReadBuf(), dataSize);
	if (processLen < 0 || dataSize < processLen || m_recvBuffer.OnReadBuffer(processLen) == false)
	{
		HandleError("ProcessRecv::OnRecv Error");
		Disconnect();
		return;
	}

	// adjust buffer cursor
	m_recvBuffer.AdjustPos();

	// register recv
	RegisterRecv();
}

/* --------------------------------------------------------
*	Method:		IocpSession::ProcessSend
*	Summary:	process completion of send request
*	Args:		int bytesTransferred
*					number of bytes sent
-------------------------------------------------------- */
void IocpSession::ProcessSend(int bytesTransferred)
{
	m_sendEvent.m_owner = nullptr;
	m_sendEvent.m_sendBuffer.clear();

	if (bytesTransferred == 0)
	{
		HandleError("ProcessSend::BytesTransferred Error");
		Disconnect();
		return;
	}

	OnSend(bytesTransferred);

	// send queue check
	m_sendBufLock.lock();
	if (m_sendBufferQueue.empty())
	{
		m_bSendRegistered.store(false);
		m_sendBufLock.unlock();
		return;
	}
	m_sendBufLock.unlock();
	
	// 만약 버퍼에 데이터가 있으면, 다시 RegisterSend 호출
	RegisterSend();
}

/* --------------------------------------------------------
*	Method:		IocpSession::OnRecv
*	Summary:	recv data processing
*	Args:		char* buffer
*					recv buffer
*				int len
*					recv data length
-------------------------------------------------------- */
int IocpSession::OnRecv(char* buffer, int len)
{
	int processLen = 0;

	while (true)
	{
		int dataSize = len - processLen;

		// packet header parsing possible?
		if (dataSize < sizeof(PacketHeader))
		{
			break;
		}	

		// packet header parsing
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));

		// packet enable?
		if (dataSize < header.size)
		{
			break;
		}

		// packet enable
		OnRecvPacket(&buffer[processLen], header.size);

		processLen += header.size;
	}
	return processLen;
}
