#include "ServerProxy.h"
#include "RioServer.h"
#include "IocpServer.h"
#include "IocpCore.h"
#include "IocpSession.h"
#include "RioSession.h"
#include "Job/GlobalQueue.h"

/* --------------------------------------------------------
*	Method:		ServerProxy::ServerProxy
*	Summary:	constructor
*	Args:		ServerType serverType
*					server type(RIO or IOCP)
*				SockAddress address
*					socket address
*				function<shared_ptr<IocpSession(void)> sessionFactory
*					session creation lambda function
-------------------------------------------------------- */
ServerProxy::ServerProxy(ServerType ServerType, SockAddress address, function<shared_ptr<IocpSession>(void)> sessionFactory)
	: m_service(nullptr)
	, m_serverType(ServerType)
{
	m_service = make_shared<IocpServer>(
		make_shared<IocpCore>(),
		sessionFactory,
		address
	);
}

/* --------------------------------------------------------
*	Method:		ServerProxy::ServerProxy
*	Summary:	constructor
*	Args:		ServerType serverType
*					server type(RIO or IOCP)
*				SockAddress address
*					socket address
*				function<shared_ptr<RioSession(void)> sessionFactory
*					session creation lambda 
-------------------------------------------------------- */
ServerProxy::ServerProxy(ServerType ServerType, SockAddress address, function<shared_ptr<RioSession>(void)> sessionFactory)
	: m_service(nullptr)
	, m_serverType(ServerType)
{
	m_service = make_shared<RioServer>(
		sessionFactory,
		address
	);
}

/* --------------------------------------------------------
*	Method:		ServerProxy::~ServerProxy
*	Summary:	destructor
-------------------------------------------------------- */
ServerProxy::~ServerProxy()
{
}

/* --------------------------------------------------------
*	Method:		ServerProxy::Start
*	Summary:	start dummy client
*	Args:		bool useJobQueue
*					whether to use job queue
-------------------------------------------------------- */
bool ServerProxy::Start(bool useJobQueue)
{
	if (m_service == nullptr)
	{
		return false;
	}

	if (m_serverType == ServerType::IOCP_SERVER)
	{
		shared_ptr<IocpServer> iocpServer = static_pointer_cast<IocpServer>(m_service);
		if (useJobQueue)
		{
			return iocpServer->Start([=]() {
					while (true)
					{
						LEndTickCount = ::GetTickCount64() + WORKER_TICK;
						iocpServer->GetIocpCore()->Dispatch(10);
						while (true)
						{
							ULONG now = ::GetTickCount64();
							if (now > LEndTickCount)
							{
								break;
							}
							shared_ptr<JobQueue> jobQueue = gGlobalQueue->Pop();
							if (jobQueue == nullptr)
							{
								break;
							}
							jobQueue->Execute();
						}
					}
				});
		}
		else
		{
			return iocpServer->Start([=]() {
					while (true)
					{
						iocpServer->GetIocpCore()->Dispatch();
					}
				});
		}
	}
	else if (m_serverType == ServerType::RIO_SERVER)
	{
		shared_ptr<RioServer> rioServer = static_pointer_cast<RioServer>(m_service);
		if (useJobQueue)
		{
			return rioServer->Start([=]() {
				shared_ptr<JobQueue> jobQueue = gGlobalQueue->Pop();
				if (jobQueue == nullptr)
				{
					return;
				}
				jobQueue->Execute();
			});
		}
		else
		{
			return rioServer->Start(nullptr);
		}
	}

	return false;
}

/* --------------------------------------------------------
*	Method:		ServerProxy::Start
*	Summary:	start dummy client
*	Args:		function<void()> logicFunc
*					logicFunction used by service
-------------------------------------------------------- */
bool ServerProxy::Start(function<void()> logicFunc)
{
	if (m_service == nullptr)
	{
		return false;
	}

	return m_service->Start(logicFunc);
}

/* --------------------------------------------------------
*	Method:		ServerProxy::Stop
*	Summary:	stop dummy client
-------------------------------------------------------- */
bool ServerProxy::Stop()
{
	if (m_service == nullptr)
	{
		return false;
	}

	return m_service->Stop();
}
