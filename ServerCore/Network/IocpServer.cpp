#include "IocpServer.h"
#include "IocpListener.h"
#include "IocpSession.h"

IocpServer::IocpServer(
	shared_ptr<IocpCore> iocpCore, 
	SessionFactory sessionFactory,
	SockAddress serverAddress, 
	int maxSessionCnt,
	int multipleThreadNum
)
	: IocpService(iocpCore, sessionFactory, serverAddress, maxSessionCnt, ServiceType::SERVER)
	, m_iocpListener(nullptr)
	, m_workerThreads()
	, m_threadCnt(0)
{
	// check number of cpus
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// set thread cnt
	m_threadCnt = si.dwNumberOfProcessors * multipleThreadNum;
}

IocpServer::~IocpServer()
{
	StopServer();
}

bool IocpServer::StartServer()
{
	if (CanStart() == false)
		return false;

	m_iocpListener = make_shared<IocpListener>(shared_from_this());
	if (m_iocpListener == nullptr)
		return false;

	// start listener
	if (m_iocpListener->StartAccept() == false)
		return false;

	// set server start status
	m_bStart = true;

	return true;
}

void IocpServer::RunServer()
{
	if (m_bStart == false)
	{
		HandleError("RunServer");
		return;
	}

	// create workerthreads
	for (int i = 0; i < m_threadCnt; i++)
	{
		m_workerThreads.push_back(thread([=]() 
			{
				while (true)
				{
					m_iocpCore->Dispatch();
				}
			}));
	}

	// TODO : server command check thread
}

void IocpServer::StopServer()
{
	// close listener socket
	m_iocpListener->CloseSocket();

	// join thread
	for (thread& thread : m_workerThreads)
	{
		if (thread.joinable() == true)
			thread.join();
	}

	// TODO : Session Stop

	// server stop
	m_bStart = false;
}


