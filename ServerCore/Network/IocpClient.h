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
	bool StartClient();
	void RunClient();
	void StopClient();

private:
	// worker threads
	vector<thread> m_workerThreads;
	int m_threadCnt;
};