#include "IocpService.h"
#include "IocpCore.h"
#include "IocpSession.h"
#include "Thread/ThreadManager.h"

/* --------------------------------------------------------
*	Method:		IocpService::IocpService
*	Summary:	constructor
*	Args:		ServiceType serviceType
*					service type(client or server)
*				shared_ptr<IocpCore> iocpCore
*					iocp core
*				IocpSessionFactory sessionFactory
*					session creation lambda function
*				SockAddress address
*					service address
-------------------------------------------------------- */
IocpService::IocpService(
	ServiceType serviceType,
	shared_ptr<IocpCore> iocpCore, 
	IocpSessionFactory sessionFactory,
	SockAddress address
)
	: m_serviceType(serviceType)
	, m_sessionFactory(sessionFactory)
	, m_serverSessions()
	, m_sessionLock()
	, m_sessionCnt(0)
	, m_iocpCore(iocpCore)
	, m_address(address)
	, m_bStart(false)
	, m_threadCnt(0)
{
	// check number of cpus
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// set thread cnt
	// threadCount = processor num * 2
	m_threadCnt = si.dwNumberOfProcessors * 2;
}

/* --------------------------------------------------------
*	Method:		IocpService::~IocpService
*	Summary:	destructor
-------------------------------------------------------- */
IocpService::~IocpService()
{
}

/* --------------------------------------------------------
*	Method:		IocpService::CreateSession
*	Summary:	create session using m_sessionFactory
-------------------------------------------------------- */
shared_ptr<IocpSession> IocpService::CreateSession()
{
	shared_ptr<IocpSession> session = m_sessionFactory();
	session->SetService(shared_from_this());
	
	if (!m_iocpCore->Register(session))
	{
		return nullptr;
	}

	return session;
}

/* --------------------------------------------------------
*	Method:		IocpService::AddSession
*	Summary:	add session to this service
*	Args:		shared_ptr<IocpSession> session
*					session to add
-------------------------------------------------------- */
void IocpService::AddSession(shared_ptr<IocpSession> session)
{
	lock_guard<mutex> lock(m_sessionLock);
	m_serverSessions.insert(session);
	m_sessionCnt++;
}

/* --------------------------------------------------------
*	Method:		IocpService::ReleaseSession
*	Summary:	release session to this service
*	Args:		shared_ptr<IocpSession> session
*					session to release
-------------------------------------------------------- */
void IocpService::ReleaseSession(shared_ptr<IocpSession> session)
{
	lock_guard<mutex> lock(m_sessionLock);
	m_serverSessions.erase(session);
	m_sessionCnt--;
}

/* --------------------------------------------------------
*	Method:		IocpService::DisconnectAllSession
*	Summary:	disconnect all session registered with the service
-------------------------------------------------------- */
void IocpService::DisconnectAllSession()
{
	lock_guard<mutex> lock(m_sessionLock);
	for (shared_ptr<IocpSession> session : m_serverSessions)
	{
		session->Disconnect();
	}
}

