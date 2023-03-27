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
	, m_iocpService(nullptr)
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
	if (m_bConnected.exchange(false) == false)
		return;

	cout << "Disconnect session" << endl;

	//OnDisconnected();
	//m_iocpService->ReleaseSession(static_pointer_cast<IocpSession>(shared_from_this()));

	RegisterDisconnect();
}

void IocpSession::Send(BYTE* buffer, int len)
{
	RegisterSend(buffer, len);
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
		ProcessSend(static_cast<SendEvent*>(iocpEvent), bytesTransferred);
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

	// set wsaBuf
	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<CHAR*>(m_recvBuffer);
	wsaBuf.len = sizeof(m_recvBuffer) / sizeof(BYTE);

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

void IocpSession::RegisterSend(BYTE* sendData, int dataLen)
{
	if (!IsConnected())
		return;

	// Init SendEvent
	// send는 recv와 달리 여러번 가능..
	SendEvent* sendEvent = new SendEvent();
	sendEvent->m_owner = shared_from_this();
	sendEvent->m_sendBuffer.resize(dataLen);
	memcpy(sendEvent->m_sendBuffer.data(), sendData, dataLen);

	// init wsaBuf
	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->m_sendBuffer.data();
	wsaBuf.len = (ULONG)sendEvent->m_sendBuffer.size();

	// WSASend
	DWORD sendLen = 0;
	if (::WSASend(m_socket, &wsaBuf, 1, &sendLen, 0, sendEvent, NULL) == SOCKET_ERROR)
	{
		if (::WSAGetLastError() != ERROR_IO_PENDING)
		{
			sendEvent->m_owner = nullptr;
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
		Disconnect();
		return;
	}

	OnRecv(m_recvBuffer, bytesTransferred);

	RegisterRecv();
}

void IocpSession::ProcessSend(SendEvent* sendEvent, int bytesTransferred)
{
	sendEvent->m_owner = nullptr;
	delete(sendEvent);

	if (bytesTransferred == 0)
	{
		Disconnect();
		return;
	}

	OnSend(bytesTransferred);

}
