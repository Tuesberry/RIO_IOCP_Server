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
	bool StartServer();
	void RunServer();
	void StopServer();

	// client Info
	void AddNewClient(int id);
	void RemoveClient(int id);
	void SetClientPos(int id, int posX, int posY);
	bool GetClientInfo(int id, CLIENT_INFO& info);

private:
	// listener
	shared_ptr<IocpListener> m_iocpListener;

	// client Info
	//map<shared_ptr<IocpSession>, CLIENT_INFO> m_clientInfo;
	mutex m_clientInfoLock;
	map<int, CLIENT_INFO> m_clientInfo;
};