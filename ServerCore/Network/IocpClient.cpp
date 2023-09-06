#include "IocpClient.h"
#include "IocpSession.h"
#include "Thread/ThreadManager.h"

/* --------------------------------------------------------
*	Method:		IocpClient::IocpClient
*	Summary:	constructor
*	Args:		shared_ptr<IocpCore> iocpCore
*					iocp core
*				IocpSessionFactory sessionFactory
*					session creation lambda function
*				SockAddress connectAddress
*					socket connect address
-------------------------------------------------------- */
IocpClient::IocpClient(
	shared_ptr<IocpCore> iocpCore, 
	IocpSessionFactory sessionFactory,
	SockAddress connectAddress
)
	:IocpService(ServiceType::CLIENT, iocpCore, sessionFactory, connectAddress)
{
}

/* --------------------------------------------------------
*	Method:		IocpClient::~IocpClient
*	Summary:	destructor
-------------------------------------------------------- */
IocpClient::~IocpClient()
{
	Stop();
}

/* --------------------------------------------------------
*	Method:		IocpClient::Start
*	Summary:	check if it can be started and start the client
-------------------------------------------------------- */
bool IocpClient::Start(function<void()> logicFunc)
{
	if (!CanStart())
	{
		HandleError("IocpClient::Start");
		return false;
	}
	
	// set client start
	m_bStart = true;

	// run client
	return RunClient(logicFunc);
}

/* --------------------------------------------------------
*	Method:		IocpClient::Stop
*	Summary:	stop client service
-------------------------------------------------------- */
bool IocpClient::Stop()
{
	// Disconnect all sessions
	DisconnectAllSession();

	// stop client
	m_bStart = false;
	return true;
}

/* --------------------------------------------------------
*	Method:		IocpClient::RunClient
*	Summary:	create worker threads and start client
-------------------------------------------------------- */
bool IocpClient::RunClient(function<void(void)> clientWork)
{
	if (!CanStart())
	{
		HandleError("IocpClient::RunClient");
		return false;
	}

	// create worker threads
	for (int i = 0; i < m_threadCnt; i++)
	{
		gThreadMgr->CreateThread(clientWork);
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		IocpClient::CreateNewSession
*	Summary:	create new session and connect to server
-------------------------------------------------------- */
bool IocpClient::ConnectNewSession()
{
	shared_ptr<IocpSession> session = CreateSession();
	return session->Connect();
}
