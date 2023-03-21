#include "IocpService.h"
#include "IocpListener.h"

mutex gMutex;

IocpService::IocpService(IocpServiceType type, SockAddress address, shared_ptr<IocpCore> core, SessionFactory factory, int maxSessionCount)
	: m_type(type)
	, m_address(address)
	, m_iocpCore(core)
	, m_sessionFactory(factory)
	, m_maxSessionCount(maxSessionCount)
{
}

IocpService::~IocpService()
{
}

bool IocpService::CanStart()
{
	return m_sessionFactory != nullptr;
}

void IocpService::CloseService()
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
	gMutex.lock();
	m_sessionCount++;
	m_sessions.insert(session);
	gMutex.unlock();
}

void IocpService::ReleaseSession(shared_ptr<IocpSession> session)
{
	gMutex.lock();
	m_sessionCount--;
	gMutex.unlock();
}

IocpClientService::IocpClientService(SockAddress targetAddress, shared_ptr<IocpCore> core, SessionFactory factory, int maxSessionCount)
	: IocpService(IocpServiceType::CLIENT, targetAddress, core, factory, maxSessionCount)
{
}

bool IocpClientService::Start()
{
	if (CanStart() == false)
		return false;

	const int sessionCount = GetMaxSessionCount();
	for (int i = 0; i < sessionCount; i++)
	{
		shared_ptr<IocpSession> session = CreateSession();
		if (session->Connect() == false)
			return false;
	}

	return true;
}

IocpServerService::IocpServerService(SockAddress targetAddress, shared_ptr<IocpCore> core, SessionFactory factory, int maxSessionCount)
	:IocpService(IocpServiceType::SERVER, targetAddress, core, factory, maxSessionCount)
{
}

bool IocpServerService::Start()
{
	if (CanStart() == false)
		return false;

	m_listener = make_shared<IocpListener>();
	if (m_listener == nullptr)
		return false;

	shared_ptr<IocpServerService> service = static_pointer_cast<IocpServerService>(shared_from_this());
	if (m_listener->StartAccept(service) == false)
		return false;

	return true;
}

void IocpServerService::CloseService()
{
	IocpService::CloseService();
}
