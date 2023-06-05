#include "RioServer.h"
#include "SocketCore.h"
#include "RioSession.h"
#include "RioCore.h"

/* --------------------------------------------------------
*	Method:		RioServer::RioServer
*	Summary:	Constructor
-------------------------------------------------------- */
RioServer::RioServer(RIOSessionFactory sessionFactory, SockAddress address)
	: m_sessionFactory(sessionFactory)
	, m_listener(INVALID_SOCKET)
	, m_address(address)
	, m_bInitListener(false)
	, m_rioCores()
	, m_bInitCore(false)
	, m_coreCnt(0)
	, m_currAllocCoreNum(0)
#if RIOIOCP
	, m_iocpHandle(nullptr)
#endif
{
}

/* --------------------------------------------------------
*	Method:		RioServer::~RioServer
*	Summary:	Destructor
-------------------------------------------------------- */
RioServer::~RioServer()
{
#if RIOIOCP
	// iocp handle close
	::CloseHandle(m_iocpHandle);
#endif
	// socket close
	SocketCore::Close(m_listener);
}

/* --------------------------------------------------------
*	Method:		RioServer::StopServer
*	Summary:	stop server
-------------------------------------------------------- */
void RioServer::StopServer()
{
	
}

/* --------------------------------------------------------
*	Method:		RioServer::InitServer
*	Summary:	initializing server
*				create and set listener and RioCore
-------------------------------------------------------- */
bool RioServer::InitServer()
{
	if (InitListener() == false)
		return false;
#if RIOIOCP
	if (CreateIocpHandle() == false)
		return false;
#endif
	if (InitCore() == false)
		return false;
	  
	return true;
}

/* --------------------------------------------------------
*	Method:		RioServer::RunServer
*	Summary:	Run server
-------------------------------------------------------- */
bool RioServer::RunServer()
{
	if (m_bInitListener == false && m_bInitCore == false)
		return false;

	// start listener
	if (StartListener() == false)
		return false;

	// start core threads
	if (StartCoreWork() == false)
		return false;

	return true;
}

/* --------------------------------------------------------
*	Method:		RioServer::InitListener
*	Summary:	initalizing listener
*				it creates listen socket, sets reuse address,
*				and binds address
-------------------------------------------------------- */
bool RioServer::InitListener()
{
	// create socket
	m_listener = SocketCore::RioSocket();
	if (m_listener == INVALID_SOCKET)
		return false;

	// set reuseaddr
	if (SocketCore::SetReuseAddr(m_listener, true) == false)
		return false;

	// bind
	if (SocketCore::Bind(m_listener, m_address) == false)
		return false;

	m_bInitListener = true;

	return true;
}

/* --------------------------------------------------------
*	Method:		RioServer::StartListener
*	Summary:	Starting listener
-------------------------------------------------------- */
bool RioServer::StartListener()
{
	// listen
	if (SocketCore::Listen(m_listener) == false)
		return false;

	// accept loop
	gThreadMgr.CreateThread([=]()
		{
			while (true)
			{
				if (Accept() == false)
					break;
			}
		});

	return true;
}

/* --------------------------------------------------------
*	Method:		RioServer::Accept
*	Summary:	accept client, create new session, and
*				allocate this sesion to specific RioCore
-------------------------------------------------------- */
bool RioServer::Accept()
{
	// create new session
	shared_ptr<RioSession> session = m_sessionFactory();

	// accept
	session->GetSocket() = SocketCore::Accept(m_listener);
	if (session->GetSocket() == INVALID_SOCKET)
	{
		HandleError("AcceptSocket");
		return false;
	}

	// get accept socket info
	SOCKADDR_IN sessionAddr;
	int addrlen = sizeof(sessionAddr);
	::getpeername(session->GetSocket(), (SOCKADDR*)&sessionAddr, &addrlen);

	// set session adddr
	session->SetAddr(SockAddress(sessionAddr));
	
	// set core
	session->SetCore(m_rioCores[m_currAllocCoreNum]);

	// allocate session in core
	m_rioCores[m_currAllocCoreNum]->AddSession(session);
	m_currAllocCoreNum = (m_currAllocCoreNum + 1) % m_coreCnt;

	// session count
	m_sessionCnt.fetch_add(1);

	// process Connect
	session->ProcessConnect();

	return true;
}

/* --------------------------------------------------------
*	Method:		RioServer::InitCore
*	Summary:	create and initialize RioCores
-------------------------------------------------------- */
bool RioServer::InitCore()
{
	// determine core count
	// = cpu core count x 2
	m_coreCnt = thread::hardware_concurrency();
	//m_coreCnt = 2;

	// allocate & init core
	for (int i = 0; i < m_coreCnt; i++)
	{
		m_rioCores.push_back(make_shared<RioCore>());
#if RIOIOCP
		if (m_rioCores[i]->InitRioCore(m_iocpHandle) == false)
		{
			HandleError("InitRioCore");
			return false;
		}
#else
		if(m_rioCores[i]->InitRioCore() == false)
		{
			HandleError("InitRioCore");
			return false;
		}
#endif
	}

	// set initCore true
	m_bInitCore = true;

	return true;
}

/* --------------------------------------------------------
*	Method:		RioServer::StartCoreWork
*	Summary:	create threads and start dispatch for work
-------------------------------------------------------- */
bool RioServer::StartCoreWork()
{
	// create thread & dispatch 
	for (int i = 0; i < m_coreCnt; i++)
	{
		gThreadMgr.CreateThread([=]() 
			{
				while (true)
				{
#if RIOIOCP
					Dispatch();
#elif SEPCQ
					m_rioCores[i]->DeferredSend();
					m_rioCores[i]->DispatchRecv();
#else
					m_rioCores[i]->DeferredSend();
					m_rioCores[i]->Dispatch();
#endif
				}
			});
	}

#if SEPCQ
	for (int i = 0; i < m_coreCnt; i++)
	{
		gThreadMgr.CreateThread([=]()
			{
				while (true)
				{
					m_rioCores[i]->DispatchSend();
				}
			});
	}
#endif

#if RIOIOCP
	// set RioNotify
	for (auto core : m_rioCores)
	{
		core->SetRioNotify();
	}
#endif
	return true;
}

/* --------------------------------------------------------
*	Method:		RioServer::Dispatch
*	Summary:	
-------------------------------------------------------- */
#if RIOIOCP
bool RioServer::Dispatch()
{
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	RioCQEvent* rioCQEvent = nullptr;

	BOOL retVal = ::GetQueuedCompletionStatus(m_iocpHandle, &bytesTransferred, &key, reinterpret_cast<LPOVERLAPPED*>(&rioCQEvent), INFINITE);

	if (retVal == TRUE)
	{
		shared_ptr<RioCore> rioCore = rioCQEvent->m_ownerCore;
#if SEPCQ
		rioCore->DispatchRecv();
		rioCore->DeferredSend();
#else
		rioCore->Dispatch();
		rioCore->DeferredSend();
#endif
	}
	else
	{
		int errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			HandleError("WAIT_TIMEOUT");
			return false;
		default:
			shared_ptr<RioCore> rioCore = rioCQEvent->m_ownerCore;
#if SEPCQ
			rioCore->DispatchRecv();
			rioCore->DeferredSend();
#else
			rioCore->Dispatch();
			rioCore->DeferredSend();
#endif			
			break;
		}
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		RioServer::CreateIocpHandle
*	Summary:	create I/O Completion Port
-------------------------------------------------------- */
bool RioServer::CreateIocpHandle()
{
	m_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	
	if (m_iocpHandle == nullptr)
	{
		HandleError("CreateIocpHandle");
		return false;
	}
	
	return true;
}
#endif