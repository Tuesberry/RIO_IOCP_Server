#pragma once

#include "Common.h"

#include "SockAddress.h"
#include "IService.h"

class IocpSession;
class IocpCore;

/* --------------------------------------------------------
*	class:		ServiceType
*	Summary:	iocp service type
-------------------------------------------------------- */
enum class ServiceType
{
	SERVER,
	CLIENT
};

using IocpSessionFactory = function<shared_ptr<IocpSession>(void)>;

/* --------------------------------------------------------
*	class:		IocpService
*	Summary:	service managed by iocp
-------------------------------------------------------- */
class IocpService : public enable_shared_from_this<IocpService>, public IService
{
public:
	IocpService(
		ServiceType serviceType,
		shared_ptr<IocpCore> iocpCore,
		IocpSessionFactory sessionFactory,
		SockAddress address
	);

	IocpService() = delete;
	IocpService(const IocpService& other) = delete;
	IocpService(IocpService&& other) = delete;
	IocpService& operator=(const IocpService& other) = delete;
	IocpService& operator=(IocpService&& other) = delete;
	virtual ~IocpService();

	// session
	shared_ptr<IocpSession> CreateSession();
	void AddSession(shared_ptr<IocpSession> session);
	void ReleaseSession(shared_ptr<IocpSession> session);
	void DisconnectAllSession();

	// get method
	shared_ptr<IocpCore> GetIocpCore() { return m_iocpCore; }
	SockAddress GetAddress() { return m_address; }
	ServiceType GetServiceType() { return m_serviceType; }
	int GetConnectCnt() { return m_sessionCnt; }
	int GetThreadCnt() { return m_threadCnt; }

	// start & stop
	bool IsStart() { return m_bStart; }
	bool CanStart() { return m_sessionFactory != nullptr; }
	
	virtual bool Start(function<void()> logicFunc) override { return false; }
	virtual bool Stop() override { return false; }

protected:
	// service type
	ServiceType m_serviceType;

	// session
	IocpSessionFactory m_sessionFactory;
	set<shared_ptr<IocpSession>> m_serverSessions;
	mutex m_sessionLock;
	atomic<int> m_sessionCnt;

	// iocp core 
	shared_ptr<IocpCore> m_iocpCore;

	// address
	SockAddress m_address;

	// start service
	bool m_bStart;

	// worker threads
	int m_threadCnt;
};