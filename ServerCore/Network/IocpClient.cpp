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

	// connect sessions
	for (int i = 0; i < m_maxSessionCnt; i++)
	{
		shared_ptr<IocpSession> session = CreateSession();
		if (session->Connect() == false)
			return false;
	}

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
