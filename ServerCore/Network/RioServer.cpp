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
{
}

/* --------------------------------------------------------
*	Method:		RioServer::~RioServer
*	Summary:	Destructor
-------------------------------------------------------- */
RioServer::~RioServer()
{
	SocketCore::Close(m_listener);
}

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
	// init listener
	if (InitListener() == false)
		return false;

	// Init Core
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
		return false;

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
	m_coreCnt = thread::hardware_concurrency() * 2;
	//m_coreCnt = 2;

	// allocate & init core
	for (int i = 0; i < m_coreCnt; i++)
	{
		m_rioCores.push_back(make_shared<RioCore>());

		if (m_rioCores[i]->InitRioCore() == false)
		{
			// TODO : Error handle
			return false;
		}
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
					m_rioCores[i]->DeferredSend();
					m_rioCores[i]->Dispatch();
				}
			});
	}

	return true;
}
