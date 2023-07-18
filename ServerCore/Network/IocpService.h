#pragma once
#include "Common.h"
#include "SockAddress.h"

class IocpSession;
class IocpCore;

/* ----------------------------
*		ServiceType
---------------------------- */
enum class ServiceType
{
	SERVER,
	CLIENT
};

/* ----------------------------
*		IocpService
---------------------------- */
using SessionFactory = function<shared_ptr<IocpSession>(void)>;

class IocpService : public enable_shared_from_this<IocpService>
{
public:
	IocpService(
		ServiceType serviceType,
		shared_ptr<IocpCore> iocpCore,
		SessionFactory sessionFactory,
		SockAddress address,
		int maxSessionCnt,
		int multipleThreadNum = 1
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
	int GetMaxSessionCnt() { return m_maxSessionCnt; }
	ServiceType GetServiceType() { return m_serviceType; }
	int GetConnectCnt() { return m_sessionCnt; }
	int GetThreadCnt() { return m_threadCnt; }

	// start
	bool IsStart() { return m_bStart; }
	bool CanStart() { return m_sessionFactory != nullptr; }
	virtual bool Start() abstract;

	// stop
	virtual bool StopService();

protected:
	// service type
	ServiceType m_serviceType;

	// session
	SessionFactory m_sessionFactory;
	set<shared_ptr<IocpSession>> m_serverSessions;
	mutex m_sessionLock;
	int m_maxSessionCnt;
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