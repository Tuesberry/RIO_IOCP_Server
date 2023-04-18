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
	: IocpService(ServiceType::SERVER, iocpCore, sessionFactory, serverAddress, maxSessionCnt, multipleThreadNum)
	, m_iocpListener(nullptr)
{
}

IocpServer::~IocpServer()
{
	StopServer();
}

bool IocpServer::InitServer()
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

void IocpServer::RunServer()
{
	if (CanStart() == false)
	{
		HandleError("RunServer");
		return;
	}

	// create threads
	CreateWorkerThreads();

	// TODO : server command check thread
}

void IocpServer::StopServer()
{
	// close listener socket
	m_iocpListener->CloseSocket();

	// DisconnectAllSessions
	DisconnectAllSession();

	// server stop
	m_bStart = false;
}

