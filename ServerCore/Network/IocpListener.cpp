#include "IocpListener.h"
#include "SocketCore.h"
#include "IocpEvent.h"
#include "SockAddress.h"
#include "IocpServer.h"
#include "IocpSession.h"

IocpListener::IocpListener(shared_ptr<IocpServer> ownServer)
	: m_listener(INVALID_SOCKET)
	, m_acceptEvents()
	, m_ownerServer(ownServer)
{
}

IocpListener::~IocpListener()
{
	CloseSocket();

	for (AcceptEvent* event : m_acceptEvents)
	{
		delete(event);
	}
}

bool IocpListener::StartAccept()
{
	// server null check
	if (m_ownerServer == nullptr)
		return false;

	// listen socket
	m_listener = SocketCore::Socket();
	if (m_listener == INVALID_SOCKET)
		return false;

	// set sockopt
	// reuse address
	if (SocketCore::SetReuseAddr(m_listener, true) == false)
		return false;

	// set linger
	if (SocketCore::SetLinger(m_listener, 0, 0) == false)
		return false;

	// bind
	if (SocketCore::Bind(m_listener, m_ownerServer->GetAddress()) == false)
		return false;

	// listen
	if (SocketCore::Listen(m_listener) == false)
		return false;

	// create accept events
	int maxSessionCnt = m_ownerServer->GetMaxSessionCnt();
	for (int i = 0; i < maxSessionCnt; i++)
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
	if (SocketCore::Close(m_listener) == false)
	{
		HandleError("IocpListener::CloseSocket");
	}
}

void IocpListener::RegisterAccept(AcceptEvent* acceptEvent)
{
	// CreateSession
	shared_ptr<IocpSession> session = m_ownerServer->CreateSession();
	
	// Init acceptEvent;
	acceptEvent->Init();
	acceptEvent->m_session = session;

	// AcceptEx
	DWORD bytesReceived = 0;
	BOOL retVal = SocketCore::AcceptEx(m_listener, session->GetSocket(), session->m_recvBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent));
	if (retVal == FALSE)
	{
		if (::WSAGetLastError() != WSA_IO_PENDING)
		{
			RegisterAccept(acceptEvent);
		}
	}
}

void IocpListener::ProcessAccept(AcceptEvent* acceptEvent)
{
	shared_ptr<IocpSession> session = acceptEvent->m_session;

	// set update accept socket
	if (SocketCore::SetUpdateAcceptSocket(session->GetSocket(), m_listener) == false)
	{
		HandleError("SetUpdateAcceptSocket");
		RegisterAccept(acceptEvent);
		return;
	}

	// get accept socket info
	SOCKADDR_IN sessionAddr;
	int addrlen = sizeof(sessionAddr);
	::getpeername(session->GetSocket(), (SOCKADDR*)&sessionAddr, &addrlen);

	// set session addr
	session->SetAddr(SockAddress(sessionAddr));

	// process connect
	session->ProcessConnect();

	// re register accept
	RegisterAccept(acceptEvent);
}

HANDLE IocpListener::GetHandle()
{
	return reinterpret_cast<HANDLE>(m_listener);
}

void IocpListener::Dispatch(IocpEvent* iocpEvent, int bytesTransferred)
{
	if (iocpEvent->m_eventType != IO_TYPE::ACCEPT)
	{
		HandleError("IocpListener::Dispatch, eventType check");
		return;
	}
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}
