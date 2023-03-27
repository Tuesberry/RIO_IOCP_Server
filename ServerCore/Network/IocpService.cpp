#include "IocpService.h"
#include "IocpCore.h"
#include "IocpSession.h"

IocpService::IocpService(
	shared_ptr<IocpCore> iocpCore, 
	SessionFactory sessionFactory, 
	SockAddress address,
	int maxSessionCnt,
	ServiceType serviceType
)
	: m_serviceType(serviceType)
	, m_sessionFactory(sessionFactory)
	, m_serverSessions()
	, m_sessionLock()
	, m_maxSessionCnt(maxSessionCnt)
	, m_sessionCnt(0)
	, m_iocpCore(iocpCore)
	, m_address(address)
	, m_bStart(false)
{
}

IocpService::~IocpService()
{
}

shared_ptr<IocpSession> IocpService::CreateSession()
{
	shared_ptr<IocpSession> session = m_sessionFactory();
	session->SetService(shared_from_this());
	
	if (m_iocpCore->Register(session) == false)
		return nullptr;

	return session;
}

void IocpService::AddSession(shared_ptr<IocpSession> session)
{
	lock_guard<mutex> lock(m_sessionLock);
	m_serverSessions.insert(session);
	m_sessionCnt++;
}

void IocpService::ReleaseSession(shared_ptr<IocpSession> session)
{
	lock_guard<mutex> lock(m_sessionLock);
	m_serverSessions.erase(session);
	m_sessionCnt--;
}