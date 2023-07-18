#pragma once
#include "Common.h"
#include "IocpService.h"

/* ----------------------------
*		IocpClient
---------------------------- */

class IocpClient : public IocpService
{
public:
	IocpClient(
		shared_ptr<IocpCore> iocpCore,
		SessionFactory sessionFactory,
		SockAddress connectAddress,
		int maxSessionCnt,
		int multipleThreadNum = 1
	);

	IocpClient() = delete;
	IocpClient(const IocpClient& other) = delete;
	IocpClient(IocpClient&& other) = delete;
	IocpClient& operator=(const IocpClient& other) = delete;
	IocpClient& operator=(IocpClient&& other) = delete;
	~IocpClient();

	// Client
	virtual bool Start() override;
	virtual bool StopService() override;

	bool RunClient(function<void(void)> clientWork);

	// Session Connect & Disconnect
	bool ConnectNewSession();
	bool DisconnectSession();

	// Session Access
	set<shared_ptr<IocpSession>>& GetSessions() { return m_serverSessions; }
};