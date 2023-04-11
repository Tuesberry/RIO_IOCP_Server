#include "RioServer.h"
#include "SocketCore.h"
#include "RioSession.h"
#include "RioCore.h"
#include "SockAddress.h"

RioServer::RioServer(SessionFactory sessionFactory)
	: m_sessionFactory(sessionFactory)
	, m_listener(INVALID_SOCKET)
	, m_bInitListener(false)
	, m_threads()
	, m_rioCores()
	, m_bInitCore(false)
	, m_coreCnt(0)
	, m_currAllocCoreNum(0)
{
}

RioServer::~RioServer()
{
	SocketCore::Close(m_listener);
}

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

bool RioServer::StartServer()
{
	if (m_bInitListener == false && m_bInitCore == false)
		return false;

	// start listener
	if (StartListener() == false)
		return false;

	// start core threads
	if (StartCoreWork() == false)
		return false;

	// join threads
	for (thread& rioThread : m_threads)
	{
		rioThread.join();
	}

	return true;
}

bool RioServer::InitListener()
{
	// create socket
	m_listener = SocketCore::Socket();
	if (m_listener == INVALID_SOCKET)
		return false;

	// set reuseaddr
	if (SocketCore::SetReuseAddr(m_listener, true) == false)
		return false;

	// bind
	if (SocketCore::BindAddrAny(m_listener, 8000) == false)
		return false;

	m_bInitListener = true;

	return true;
}

bool RioServer::StartListener()
{
	// listen
	if (SocketCore::Listen(m_listener) == false)
		return false;

	// accept loop
	m_threads.push_back(::thread([=]()
		{
			while (true)
			{
				if (Accept() == false)
					break;
			}
		}));

	return true;
}

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

	// allocate session to core
	m_rioCores[m_currAllocCoreNum]->AddSession(session);
	m_currAllocCoreNum = (m_currAllocCoreNum + 1) % m_coreCnt;

	// process Connect
	session->ProcessConnect();

	return true;
}

bool RioServer::InitCore()
{
	// determine core count
	// = cpu core count x 2
	m_coreCnt = thread::hardware_concurrency() * 2;

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

bool RioServer::StartCoreWork()
{
	// create thread & dispatch 
	for (int i = 0; i < m_coreCnt; i++)
	{
		m_threads.push_back(::thread([=]() 
			{
				while (true)
				{
					m_rioCores[i]->Dispatch();
				}
			}));
	}

	return true;
}
