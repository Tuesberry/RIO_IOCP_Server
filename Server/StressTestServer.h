#pragma once
#include "CoreCommon.h"
#include "Network/IocpServer.h"

class StressTestServer
{
	StressTestServer() = delete;
	StressTestServer(shared_ptr<IocpServer> server);

	StressTestServer(const StressTestServer& other) = delete;
	StressTestServer(StressTestServer&& other) = delete;
	StressTestServer& operator=(const StressTestServer& other) = delete;
	StressTestServer& operator=(StressTestServer&& other) = delete;
	~StressTestServer();

	void RunServer();
private:
	void Draw();

private:
	shared_ptr<IocpServer> m_server;

};