#include "IocpClient.h"
#include "IocpSession.h"

IocpClient::IocpClient(
	shared_ptr<IocpCore> iocpCore, 
	SessionFactory sessionFactory, 
	SockAddress connectAddress, 
	int maxSessionCnt, 
	int multipleThreadNum
)
	:IocpService(ServiceType::CLIENT,iocpCore, sessionFactory, connectAddress, maxSessionCnt, multipleThreadNum)
	, m_lastConnectTime()
	, m_bCanConnected(true)
{
}

IocpClient::~IocpClient()
{
	StopClient();
}

bool IocpClient::StartClient()
{
	if (CanStart() == false)
		return false;

	// set client start
	m_bStart = true;

	return true;
}

void IocpClient::RunClient()
{
	if (CanStart() == false)
		return;

	// create worker threads
	CreateWorkerThreads();
}

void IocpClient::StopClient()
{
	// Disconnect all sessions
	DisconnectAllSession();

	// join threads
	JoinWorkerThreads();

	// stop client
	m_bStart = false;
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

