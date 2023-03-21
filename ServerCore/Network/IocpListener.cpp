#include "IocpListener.h"
#include "SocketCore.h"
#include "IocpEvent.h"
#include "IocpSession.h"
#include "IocpService.h"

IocpListener::IocpListener()
	: m_socket(INVALID_SOCKET)
	, m_acceptEvents()
{
}

IocpListener::~IocpListener()
{
	SocketCore::Close(m_socket);

	for (AcceptEvent* event : m_acceptEvents)
	{
		delete(event);
	}
}

bool IocpListener::StartAccept(shared_ptr<IocpServerService> service)
{
	m_service = service;
	if (m_service == nullptr)
		return false;

	m_socket = SocketCore::Socket();
	if (m_socket == INVALID_SOCKET)
		return false;

	if (m_service->GetIocpCore()->Register(shared_from_this()) == false)
		return false;

	if (SocketCore::SetReuseAddr(m_socket, true) == false)
		return false;

	if (SocketCore::SetLinger(m_socket, 0, 0) == false)
		return false;

	if (SocketCore::Bind(m_socket, m_service->GetAddress()) == false)
		return false;

	if (SocketCore::Listen(m_socket) == false)
		return false;	

	const int acceptCount = m_service->GetMaxSessionCount();
	for (int i = 0; i < acceptCount; i++)
	{
		AcceptEvent* acceptEvent = new AcceptEvent();
		acceptEvent->m_owner = shared_from_this();
		m_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

void IocpListener::CloseSocket()
{
	SocketCore::Close(m_socket);
}

HANDLE IocpListener::GetHandle()
{
	return reinterpret_cast<HANDLE>(m_socket);
}

void IocpListener::ExecuteTask(IocpEvent* iocpEvent, int bytesTransferred)
{
	if (iocpEvent->m_eventType != EventType::ACCEPT)
		return;

	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void IocpListener::RegisterAccept(AcceptEvent* acceptEvent)
{
	shared_ptr<IocpSession> session = m_service->CreateSession(); // Register IOCP

	acceptEvent->Init();
	acceptEvent->m_session = session;

	DWORD bytesReceived = 0;
	if (false == SocketCore::AcceptEx(m_socket, session->GetSocket(), session->m_recvBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			// 일단 다시 Accept 걸어준다
			RegisterAccept(acceptEvent);
		}
	}
}

void IocpListener::ProcessAccept(AcceptEvent* acceptEvent)
{
	shared_ptr<IocpSession> session = acceptEvent->m_session;

	if (false == SocketCore::SetUpdateAcceptSocket(session->GetSocket(), m_socket))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	session->SetAddress(SockAddress(sockAddress));
	session->ProcessConnect();
	RegisterAccept(acceptEvent);
}
