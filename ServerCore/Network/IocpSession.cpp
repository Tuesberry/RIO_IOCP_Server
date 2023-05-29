#include "IocpSession.h"
#include "SocketCore.h"
#include "IocpService.h"

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
		HandleError("IocpSession::IocpSession");
}

IocpSession::~IocpSession()
{
	SocketCore::Close(m_socket);
}

bool IocpSession::Connect()
{
	return RegisterConnect();
}

void IocpSession::Disconnect()
{
	cout << "Disconnect Called" << endl;

	if (m_bConnected.exchange(false) == false)
	{
		return;
	}
	RegisterDisconnect();
}

void IocpSession::Send(shared_ptr<SendBuffer> sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registered = false;
	{
		m_sendBufLock.lock();

		// enqueue sendbuffer
		m_sendBufferQueue.push(sendBuffer);

		if (m_bSendRegistered.exchange(true) == false)
			registered = true;

		m_sendBufLock.unlock();
	}

	if (registered)
		RegisterSend();
}

HANDLE IocpSession::GetHandle()
{
	return reinterpret_cast<HANDLE>(m_socket);
}

void IocpSession::Dispatch(IocpEvent* iocpEvent, int bytesTransferred)
{
	IO_TYPE ioType = iocpEvent->m_eventType;
	switch (ioType)
	{
	case IO_TYPE::CONNECT:
		ProcessConnect();
		break;
	case IO_TYPE::DISCONNECT:
		ProcessDisconnect();
		break;
	case IO_TYPE::RECV:
		ProcessRecv(bytesTransferred);
		break;
	case IO_TYPE::SEND:
		ProcessSend(bytesTransferred);
		break;
	default:
		HandleError("IocpSession::Dispatch");
		break;
	}
}

bool IocpSession::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (m_iocpService->GetServiceType() != ServiceType::CLIENT)
		return false;

	// set reuse address
	if (SocketCore::SetReuseAddr(m_socket, true) == false)
		return false;

	// bind any address
	if (SocketCore::BindAddrAny(m_socket, 0) == false)
		return false;

	// init connect event
	m_connectEvent.Init();
	m_connectEvent.m_owner = shared_from_this();

	// conenctEx
	DWORD bytesTransferred = 0;
	SOCKADDR_IN sockAddr = m_iocpService->GetAddress().GetSockAddr();
	if (SocketCore::ConnectEx(m_socket, (SOCKADDR*)(&sockAddr), sizeof(sockAddr), nullptr, 0, &bytesTransferred, &m_connectEvent) == false)
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

void IocpSession::RegisterDisconnect()
{
	m_disconnectEvent.Init();
	m_disconnectEvent.m_owner = shared_from_this();

	if (SocketCore::DisconnectEx(m_socket, &m_disconnectEvent, TF_REUSE_SOCKET, 0) == false)
	{
		if (::WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_disconnectEvent.m_owner = nullptr;
			HandleError("DisconnectEx");
			return;
		}
	}
}

void IocpSession::RegisterRecv()
{
	if (!IsConnected())
		return;

	// init recvEvent
	m_recvEvent.Init();
	m_recvEvent.m_owner = shared_from_this();

	// log
	if (m_recvBuffer.GetFreeSize() == 0)
		cout << "Recv Buffer Free Size == 0" << endl;

	// set wsaBuf
	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<CHAR*>(m_recvBuffer.GetWritePos());
	wsaBuf.len = m_recvBuffer.GetFreeSize();

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

void IocpSession::RegisterSend()
{
	if (!IsConnected())
		return;

	// Scatter-Gather
	// init m_sendEvent
	m_sendEvent.Init();
	m_sendEvent.m_owner = shared_from_this();
	// add sendBuffer in sendEvent
	{
		m_sendBufLock.lock();

		while (m_sendBufferQueue.empty() == false)
		{
			shared_ptr<SendBuffer> sendBuf = m_sendBufferQueue.front();
			m_sendBufferQueue.pop();

			m_sendEvent.m_sendBuffer.push_back(sendBuf);
		}

		m_sendBufLock.unlock();
	}
	// add all sendData in WSABuf
	vector<WSABUF> wsaBuf;
	wsaBuf.reserve(m_sendEvent.m_sendBuffer.size());
	for (shared_ptr<SendBuffer> sendBuf : m_sendEvent.m_sendBuffer)
	{
		WSABUF buf;
		buf.buf = (char*)sendBuf->GetData();
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

void IocpSession::ProcessDisconnect()
{
	m_disconnectEvent.m_owner = nullptr;

	OnDisconnected();

	m_iocpService->ReleaseSession(static_pointer_cast<IocpSession>(shared_from_this()));
}

void IocpSession::ProcessRecv(int bytesTransferred)
{
	m_recvEvent.m_owner = nullptr;

	if (bytesTransferred == 0)
	{
		if (::WSAGetLastError() == ERROR_IO_PENDING)
		{
			cout << "IO PENDING" << endl;
			return;
		}
		HandleError("ProcessRecv::ByteTransferred Error");
		Disconnect();
		return;
	}

	if (m_recvBuffer.OnWrite(bytesTransferred) == false)
	{
		HandleError("ProcessRecv::OnWrite Error");
		Disconnect();
		return;
	}

	// on recv
	int dataSize = m_recvBuffer.GetDataSize();
	int processLen = OnRecv(m_recvBuffer.GetReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || m_recvBuffer.OnRead(processLen) == false)
	{
		HandleError("ProcessRecv::OnRecv Error");
		Disconnect();
		return;
	}

	// adjust cursor
	m_recvBuffer.AdjustPos();

	// register recv
	RegisterRecv();
}

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
	RegisterSend();
}

int IocpSession::OnRecv(BYTE* buffer, int len)
{
	int processLen = 0;

	while (true)
	{
		int dataSize = len - processLen;

		// packet header parsing possible?
		if (dataSize < sizeof(PacketHeader))
		{
			if (dataSize > BUFSIZE)
			{
				cout << "OnRecv Data Size = " << dataSize << endl;
			}
			break;
		}	

		// packet header parsing
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));

		// packet enable?
		if (dataSize < header.size)
		{
			if (dataSize > BUFSIZE)
			{
				cout << "OnRecv Data Size = " << dataSize << " Header Size = " << header.size << endl;
			}
			//cout << "OnRecv Data Size = " << dataSize << " Header Size = " << header.size << endl;
			break;
		}

		// packet enable
		OnRecvPacket(&buffer[processLen], header.size);

		processLen += header.size;

	}

	return processLen;
}
