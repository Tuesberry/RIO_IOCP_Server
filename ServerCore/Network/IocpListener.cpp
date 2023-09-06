#include "IocpListener.h"
#include "SocketCore.h"
#include "IocpEvent.h"
#include "SockAddress.h"
#include "IocpServer.h"
#include "IocpSession.h"

/* --------------------------------------------------------
*	Method:		IocpListener::IocpListener
*	Summary:	constructor
*	Args:		shared_ptr<IocpServer> ownServer
*					server that owns this listener
-------------------------------------------------------- */
IocpListener::IocpListener(shared_ptr<IocpServer> ownServer)
	: m_listener(INVALID_SOCKET)
	, m_acceptEvents()
	, m_ownerServer(ownServer)
{
}

/* --------------------------------------------------------
*	Method:		IocpListener::~IocpListener
*	Summary:	destructor
-------------------------------------------------------- */
IocpListener::~IocpListener()
{
	CloseSocket();

	for (AcceptEvent* event : m_acceptEvents)
	{
		delete(event);
	}
}

/* --------------------------------------------------------
*	Method:		IocpListener::StartAccept
*	Summary:	start accept
-------------------------------------------------------- */
bool IocpListener::StartAccept()
{
	// server null check
	if (m_ownerServer == nullptr)
	{
		return false;
	}

	// create listen socket
	m_listener = SocketCore::Socket();
	if (m_listener == INVALID_SOCKET)
	{
		return false;
	}

	// register listener
	if (!m_ownerServer->GetIocpCore()->Register(shared_from_this()))
	{
		return false;
	}

	// set sockopt
	// reuse address
	if (!SocketCore::SetReuseAddr(m_listener, true))
	{
		return false;
	}

	// set linger
	if (!SocketCore::SetLinger(m_listener, 0, 0))
	{
		return false;
	}

	// bind
	if (!SocketCore::Bind(m_listener, m_ownerServer->GetAddress()))
	{
		return false;
	}

	// listen
	if (!SocketCore::Listen(m_listener))
	{
		return false;
	}

	// create accept events
	int maxSessionCnt = m_ownerServer->GetThreadCnt();
	for (int i = 0; i < maxSessionCnt; i++)
	{
		AcceptEvent* acceptEvent = new AcceptEvent();
		acceptEvent->m_owner = shared_from_this();
		m_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		IocpListener::CloseSocket
*	Summary:	close socket
-------------------------------------------------------- */
void IocpListener::CloseSocket()
{
	if (!SocketCore::Close(m_listener))
	{
		HandleError("IocpListener::CloseSocket");
	}
}

/* --------------------------------------------------------
*	Method:		IocpListener::RegisterAccept
*	Summary:	register accept request
*	Args:		AcceptEvent* acceptEvent
*					Event for accept request
-------------------------------------------------------- */
void IocpListener::RegisterAccept(AcceptEvent* acceptEvent)
{
	// CreateSession
	shared_ptr<IocpSession> session = m_ownerServer->CreateSession();
	
	// Init acceptEvent;
	acceptEvent->Init();
	acceptEvent->m_session = session;

	// AcceptEx
	DWORD bytesReceived = 0;
	BOOL retVal = SocketCore::AcceptEx(m_listener, session->GetSocket(), session->m_recvBuffer.GetWriteBuf(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent));
	if (retVal == FALSE)
	{
		if (::WSAGetLastError() != WSA_IO_PENDING)
		{
			RegisterAccept(acceptEvent);
		}
	}
}

/* --------------------------------------------------------
*	Method:		IocpListener::RegisterAccept
*	Summary:	process completion of accept request
*	Args:		AcceptEvent* acceptEvent
*					Event for accept request
-------------------------------------------------------- */
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

/* --------------------------------------------------------
*	Method:		IocpListener::GetHandle
*	Summary:	return listener socket
-------------------------------------------------------- */
HANDLE IocpListener::GetHandle()
{
	return reinterpret_cast<HANDLE>(m_listener);
}

/* --------------------------------------------------------
*	Method:		IocpListener::Dispatch
*	Summary:	dispatch accept event
*	Args:		IocpEvent* iocpEvent
*					Event for io request
*				int bytesTransferred
*					number of bytes trasferred
-------------------------------------------------------- */
void IocpListener::Dispatch(IocpEvent* iocpEvent, int bytesTransferred)
{
	if (iocpEvent->m_eventType != IOCP_IO_TYPE::ACCEPT)
	{
		HandleError("IocpListener::Dispatch, eventType check");
		return;
	}
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}
