#pragma once

#include "Common.h"

#include "IService.h"
#include "SockAddress.h"

class IocpSession;
class RioSession;

/* --------------------------------------------------------
*	class:		ServerType
*	Summary:	network Server type
-------------------------------------------------------- */
enum class ServerType
{
	IOCP_SERVER,
	RIO_SERVER
};

/* --------------------------------------------------------
*	class:		ClientProxy
*	Summary:	network service proxy class
-------------------------------------------------------- */
class ServerProxy : public IService
{
	enum ServerConfig
	{
		WORKER_TICK = 64
	};

public:
	ServerProxy(
		ServerType ServerType,
		SockAddress address,
		function<shared_ptr<IocpSession>(void)> sessionFactory
	);

	ServerProxy(
		ServerType ServerType,
		SockAddress address,
		function<shared_ptr<RioSession>(void)> sessionFactory
	);

	ServerProxy() = delete;
	ServerProxy(const ServerProxy& other) = delete;
	ServerProxy(ServerProxy&& other) = delete;
	ServerProxy& operator=(const ServerProxy& other) = delete;
	ServerProxy& operator=(ServerProxy&& other) = delete;
	virtual ~ServerProxy();

	bool Start(bool useJobQueue);
	virtual bool Start(function<void()> logicFunc) override;
	virtual bool Stop() override;

private:
	shared_ptr<IService> m_service;
	ServerType m_serverType;
};