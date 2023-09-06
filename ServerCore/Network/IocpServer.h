#pragma once

#include "Common.h"

#include "IocpService.h"

class IocpListener;

/* --------------------------------------------------------
*	class:		IocpServer
*	Summary:	server managed by IOCP
-------------------------------------------------------- */
class IocpServer : public IocpService
{
public:
	IocpServer(
		shared_ptr<IocpCore> iocpCore, 
		IocpSessionFactory sessionFactory,
		SockAddress serverAddress
	);

	IocpServer() = delete;
	IocpServer(const IocpServer& other) = delete;
	IocpServer(IocpServer&& other) = delete;
	IocpServer& operator=(const IocpServer& other) = delete;
	IocpServer& operator=(IocpServer&& other) = delete;
    virtual ~IocpServer();

	virtual bool Start(function<void()> logicFunc) override;
	virtual bool Stop() override;

private:
	bool RunServer(function<void(void)> serverWork);

private:
	shared_ptr<IocpListener> m_iocpListener;
};