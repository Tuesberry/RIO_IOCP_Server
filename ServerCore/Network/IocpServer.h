#pragma once
#include "Common.h"
#include "IocpService.h"

class IocpListener;

/* ----------------------------
*		ClientInfo
---------------------------- */
struct CLIENT_INFO
{
	int posX;
	int posY;
};

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

	// server
	bool InitServer();
	void RunServer();
	void StopServer();

private:
	// listener
	shared_ptr<IocpListener> m_iocpListener;
};