#include "IocpClient.h"
#include "IocpSession.h"
#include "Thread/ThreadManager.h"

IocpClient::IocpClient(
	shared_ptr<IocpCore> iocpCore, 
	SessionFactory sessionFactory, 
	SockAddress connectAddress, 
	int maxSessionCnt, 
	int multipleThreadNum
)
	:IocpService(ServiceType::CLIENT,iocpCore, sessionFactory, connectAddress, maxSessionCnt, multipleThreadNum)
{
}

IocpClient::~IocpClient()
{
	StopService();
}

bool IocpClient::Start()
{
	if (CanStart() == false)
		return false;

	// set client start
	m_bStart = true;

	return true;
}

bool IocpClient::StopService()
{
	// Disconnect all sessions
	DisconnectAllSession();

	// stop client
	m_bStart = false;

	return true;
}

bool IocpClient::RunClient(function<void(void)> clientWork)
{
	if (CanStart() == false)
	{
		HandleError("RunClient");
		return false;
	}

	// create worker threads
	for (int i = 0; i < m_threadCnt; i++)
	{
		gThreadMgr->CreateThread(clientWork);
	}

	return true;
}

bool IocpClient::ConnectNewSession()
{
	shared_ptr<IocpSession> session = CreateSession();
	return session->Connect();
}

bool IocpClient::DisconnectSession()
{
	lock_guard<mutex> lock(m_sessionLock);
	set<shared_ptr<IocpSession>>::iterator it = m_serverSessions.begin();
	(*it)->Disconnect();
	return true;
}

