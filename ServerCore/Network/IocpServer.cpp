#include "IocpServer.h"
#include "IocpListener.h"
#include "IocpSession.h"
#include "Thread/ThreadManager.h"

IocpServer::IocpServer(
	shared_ptr<IocpCore> iocpCore, 
	SessionFactory sessionFactory,
	SockAddress serverAddress, 
	int maxSessionCnt,
	int multipleThreadNum
)
	: IocpService(ServiceType::SERVER, iocpCore, sessionFactory, serverAddress, maxSessionCnt, multipleThreadNum)
	, m_iocpListener(nullptr)
{
}

IocpServer::~IocpServer()
{
	StopService();
}

bool IocpServer::Start()
{
	if (CanStart() == false)
		return false;

	m_iocpListener = make_shared<IocpListener>(static_pointer_cast<IocpServer>(shared_from_this()));
	if (m_iocpListener == nullptr)
		return false;

	// start listener
	if (m_iocpListener->StartAccept() == false)
		return false;

	// set server start status
	m_bStart = true;

	return true;
}

bool IocpServer::StopService()
{
	// close listener socket
	m_iocpListener->CloseSocket();

	// DisconnectAllSessions
	DisconnectAllSession();

	// server stop
	m_bStart = false;

	return true;
}

bool IocpServer::RunServer(function<void(void)> serverWork)
{
	if (CanStart() == false)
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