#pragma once

#include "Common.h"

#include "IocpService.h"

/* --------------------------------------------------------
*	class:		IocpClient
*	Summary:	client managed by IOCP
-------------------------------------------------------- */
class IocpClient : public IocpService
{
public:
	IocpClient(
		shared_ptr<IocpCore> iocpCore,
		IocpSessionFactory sessionFactory,
		SockAddress connectAddress
	);

	IocpClient() = delete;
	IocpClient(const IocpClient& other) = delete;
	IocpClient(IocpClient&& other) = delete;
	IocpClient& operator=(const IocpClient& other) = delete;
	IocpClient& operator=(IocpClient&& other) = delete;
	virtual ~IocpClient();
	
	virtual bool Start(function<void()> logicFunc) override;
	virtual bool Stop() override;

	bool ConnectNewSession();

private:
	bool RunClient(function<void(void)> clientWork);
};