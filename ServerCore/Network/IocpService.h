#pragma once
#include "Common.h"
#include "IocpListener.h"
#include "IocpCore.h"
#include "IocpSession.h"
#include "SockAddress.h"
#include <functional>

enum class IocpServiceType : UINT8
{
	SERVER,
	CLIENT
};

/* ----------------------------
*		Service
---------------------------- */

using SessionFactory = function<shared_ptr<IocpSession>(void)>;

class IocpService : public enable_shared_from_this<IocpService>
{
public:
	IocpService(IocpServiceType type, SockAddress address, shared_ptr<IocpCore> core, SessionFactory factory, int maxSessionCount = 1);

	IocpService() = delete;
	IocpService(const IocpService& other) = delete;
	IocpService(IocpService&& other) = delete;
	IocpService& operator=(const IocpService& other) = delete;
	IocpService& operator=(IocpService&& other) = delete;
	
	virtual ~IocpService();

	virtual bool Start() abstract;
	bool CanStart();

	virtual void CloseService();
	void SetSessionFactory(SessionFactory func) { m_sessionFactory = func; }

	shared_ptr<IocpSession>	CreateSession();
	void AddSession(shared_ptr<IocpSession> session);
	void ReleaseSession(shared_ptr<IocpSession> session);
	int	GetCurrentSessionCount() { return m_sessionCount; }
	int	GetMaxSessionCount() { return m_maxSessionCount; }

public:
	IocpServiceType	GetServiceType() { return m_type; }
	SockAddress GetAddress() { return m_address; }
	shared_ptr<IocpCore>& GetIocpCore() { return m_iocpCore; }

protected:
	IocpServiceType m_type;
	SockAddress m_address = {};
	shared_ptr<IocpCore> m_iocpCore;

	set<shared_ptr<IocpSession>> m_sessions;
	int m_sessionCount;
	int m_maxSessionCount;
	SessionFactory m_sessionFactory;
};

/*-----------------
	ClientService
------------------*/

class IocpClientService : public IocpService
{
public:
	IocpClientService(SockAddress targetAddress, shared_ptr<IocpCore> core, SessionFactory factory, int maxSessionCount = 1);
	virtual ~IocpClientService() {}

	virtual bool Start() override;
};

/*-----------------
	ServerService
------------------*/

class IocpServerService : public IocpService
{
public:
	IocpServerService(SockAddress targetAddress, shared_ptr<IocpCore> core, SessionFactory factory, int maxSessionCount = 1);
	virtual ~IocpServerService() {}

	virtual bool Start() override;
	virtual void CloseService() override;

private:
	shared_ptr<IocpListener>	m_listener = nullptr;
};