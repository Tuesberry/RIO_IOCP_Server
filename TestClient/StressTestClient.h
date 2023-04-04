#pragma once
#include "CoreCommon.h"
#include "Network/IocpClient.h"

class StressTestClient
{
	enum {
		CONNECT_TIME_LIMIT_MS = 100000,
	};
public:
	StressTestClient() = delete;
	StressTestClient(shared_ptr<IocpClient> client);

	StressTestClient(const StressTestClient& other) = delete;
	StressTestClient(StressTestClient&& other) = delete;
	StressTestClient& operator=(const StressTestClient& other) = delete;
	StressTestClient& operator=(StressTestClient&& other) = delete;
	~StressTestClient();

	void RunServer();

private:
	void UpdateConnection();

	void InitOutput();
	void UpdateOutput();

	void MoveCursor(int x, int y);

private:
	shared_ptr<IocpClient> m_client;

	high_resolution_clock::time_point m_connectStartTime;
	duration<long long, micro> m_lastConnectTime;

	COORD m_initCursor;
};