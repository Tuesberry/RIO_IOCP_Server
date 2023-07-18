#pragma once
#include "Common.h"
#include "IocpService.h"

class IocpListener;

/* ----------------------------
*		IocpServer
---------------------------- */

class IocpServer : public IocpService
{
public:
	IocpServer(
		shared_ptr<IocpCore> iocpCore, 
		SessionFactory sessionFactory, 
		SockAddress serverAddress, 
		int maxSessionCnt,
		int multipleThreadNum = 1
	);

	IocpServer() = delete;
	IocpServer(const IocpServer& other) = delete;
	IocpServer(IocpServer&& other) = delete;
	IocpServer& operator=(const IocpServer& other) = delete;
	IocpServer& operator=(IocpServer&& other) = delete;
     ~IocpServer();

	virtual bool Start() override;
	virtual bool StopService() override;

	bool RunServer(function<void(void)> serverWork);

private:
	shared_ptr<IocpListener> m_iocpListener;
};