#include "IocpServer.h"
#include "IocpListener.h"
#include "IocpSession.h"
#include "Thread/ThreadManager.h"

/* --------------------------------------------------------
*	Method:		IocpServer::IocpServer
*	Summary:	constructor
*	Args:		shared_ptr<IocpCore> iocpCore
*					iocp core
*				IocpSessionFactory sessionFactory
*					session creation lambda function
*				SockAddress serverAddress
*					server socket address
-------------------------------------------------------- */
IocpServer::IocpServer(
	shared_ptr<IocpCore> iocpCore, 
	IocpSessionFactory sessionFactory,
	SockAddress serverAddress
)
	: IocpService(ServiceType::SERVER, iocpCore, sessionFactory, serverAddress)
	, m_iocpListener(nullptr)
{
}

/* --------------------------------------------------------
*	Method:		IocpServer::~IocpServer
*	Summary:	destructor
-------------------------------------------------------- */
IocpServer::~IocpServer()
{
	Stop();
}

/* --------------------------------------------------------
*	Method:		IocpServer::Start
*	Summary:	create listener and start server 
-------------------------------------------------------- */
bool IocpServer::Start(function<void()> logicFunc)
{
	if (!CanStart())
	{
		return false;
	}

	m_iocpListener = make_shared<IocpListener>(static_pointer_cast<IocpServer>(shared_from_this()));
	if (m_iocpListener == nullptr)
	{
		return false;
	}

	// start listener
	if (!m_iocpListener->StartAccept())
	{
		return false;
	}

	// set server start status
	m_bStart = true;

	return RunServer(logicFunc);
}

/* --------------------------------------------------------
*	Method:		IocpServer::Stop
*	Summary:	close socket and disconnect all session
-------------------------------------------------------- */
bool IocpServer::Stop()
{
	// close listener socket
	m_iocpListener->CloseSocket();

	// DisconnectAllSessions
	DisconnectAllSession();

	// server stop
	m_bStart = false;

	return true;
}

/* --------------------------------------------------------
*	Method:		IocpServer::RunServer
*	Summary:	create worker threads and start server
-------------------------------------------------------- */
bool IocpServer::RunServer(function<void(void)> serverWork)
{
	if (!CanStart())
	{
		HandleError("RunServer");
		return false;
	}

	// create threads
	for (int i = 0; i < m_threadCnt; i++)
	{
		gThreadMgr->CreateThread(serverWork);
	}

	return true;
}