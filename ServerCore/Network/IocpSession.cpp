#include "IocpSession.h"
#include "IocpService.h"
#include "SocketCore.h"

IocpSession::IocpSession()
{
	m_socket = SocketCore::Socket();
}

IocpSession::~IocpSession()
{
	SocketCore::Close(m_socket);
}

void IocpSession::Send(BYTE* buffer, int len)
{
	SendEvent* sendEvent = new SendEvent();
	sendEvent->m_owner = shared_from_this(); // ADD_REF
	sendEvent->m_buffer.resize(len);
	::memcpy(sendEvent->m_buffer.data(), buffer, len);
	RegisterSend(sendEvent);
}

bool IocpSession::Connect()
{
	return RegisterConnect();
}

void IocpSession::Disconnect(const WCHAR* cause)
{
	if (m_connected.exchange(false) == false)
		return;

	cout << "Disconnect : " << cause << endl;

	OnDisconnected(); // 컨텐츠 코드에서 재정의
	GetService()->ReleaseSession(GetSessionRef());

	RegisterDisconnect();
}

HANDLE IocpSession::GetHandle()
{
	return reinterpret_cast<HANDLE>(m_socket);
}

void IocpSession::ExecuteTask(IocpEvent* iocpEvent, int bytesTrasferred)
{
	switch (iocpEvent->m_eventType)
	{
	case EventType::CONNECT:
		ProcessConnect();
		break;
	case EventType::DISCONNECT:
		ProcessDisconnect();
		break;
	case EventType::RECV:
		ProcessRecv(bytesTrasferred);
		break;
	case EventType::SEND:
		ProcessSend(static_cast<SendEvent*>(iocpEvent), bytesTrasferred);
		break;
	default:
		break;
	}
}

bool IocpSession::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != IocpServiceType::CLIENT)
		return false;

	if (SocketCore::SetReuseAddr(m_socket, true) == false)
		return false;

	if (SocketCore::BindAddrAny(m_socket, 0/*남는거*/) == false)
		return false;

	m_connectEvent.Init();
	m_connectEvent.m_owner = shared_from_this(); // ADD_REF

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetAddress().GetSockAddr();
	if (false == SocketCore::ConnectEx(m_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &m_connectEvent))
	{
		int errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			m_connectEvent.m_owner = nullptr; // RELEASE_REF
			return false;
		}
	}

	return true;
}

bool IocpSession::RegisterDisconnect()
{
	m_disconnectEvent.Init();
	m_disconnectEvent.m_owner = shared_from_this(); // ADD_REF

	if (false == SocketCore::DisconnectEx(m_socket, &m_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			m_disconnectEvent.m_owner = nullptr; // RELEASE_REF
			return false;
		}
	}

	return true;
}

void IocpSession::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	m_recvEvent.Init();
	m_recvEvent.m_owner = shared_from_this(); // ADD_REF

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(m_recvBuffer);
	wsaBuf.len = sizeof(m_recvBuffer) / sizeof(m_recvBuffer[0]);

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(m_socket, &wsaBuf, 1, OUT & numOfBytes, OUT & flags, &m_recvEvent, nullptr))
	{
		int errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			m_recvEvent.m_owner = nullptr; // RELEASE_REF
		}
	}
}

void IocpSession::RegisterSend(SendEvent* sendEvent)
{
	if (IsConnected() == false)
		return;

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->m_buffer.data();
	wsaBuf.len = (ULONG)sendEvent->m_buffer.size();

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == ::WSASend(m_socket, &wsaBuf, 1, OUT & numOfBytes, 0, sendEvent, nullptr))
	{
		int errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			sendEvent->m_owner = nullptr; // RELEASE_REF
			delete(sendEvent);
		}
	}
}

void IocpSession::ProcessConnect()
{
	m_connectEvent.m_owner = nullptr; // RELEASE_REF

	m_connected.store(true);

	// 세션 등록
	GetService()->AddSession(GetSessionRef());

	// 컨텐츠 코드에서 재정의
	OnConnected();

	// 수신 등록
	RegisterRecv();
}

void IocpSession::ProcessDisconnect()
{
	m_disconnectEvent.m_owner = nullptr; // RELEASE_REF
}

void IocpSession::ProcessRecv(int numOfBytes)
{
	m_recvEvent.m_owner = nullptr; // RELEASE_REF

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	// 컨텐츠 코드에서 재정의
	OnRecv(m_recvBuffer, numOfBytes);

	// 수신 등록
	RegisterRecv();
}

void IocpSession::ProcessSend(SendEvent* sendEvent, int numOfBytes)
{
	sendEvent->m_owner = nullptr; // RELEASE_REF
	delete(sendEvent);

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	// 컨텐츠 코드에서 재정의
	OnSend(numOfBytes);
}

void IocpSession::HandleError(int errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		// TODO : Log
		cout << "Handle Error : " << errorCode << endl;
		break;
	}
}
