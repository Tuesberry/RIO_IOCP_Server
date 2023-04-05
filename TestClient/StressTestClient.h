#pragma once
#include "CoreCommon.h"
#include "Network/IocpClient.h"

class DelayManager
{
public:
	enum
	{
		CONNECT_DELAY_INIT_MICROS = 10000,
		DELAY_LIMIT_MS = 100,
		AVG_DELAY_LIMIT_MS = 100,
	};

	DelayManager();
	DelayManager(const DelayManager& other) = delete;
	DelayManager(DelayManager&& other) = delete;
	DelayManager& operator=(const DelayManager& other) = delete;
	DelayManager& operator=(DelayManager&& other) = delete;
	~DelayManager() = default;

	void UpdateDelay(int delay);

	void UpdateAvgDelay(int delay, int prevDelay);
	void AddNewInAvgDelay(int delay);
	void DeleteInAvgDelay(int delay);

	void UpdateConnectionStatus();

public:
	unsigned int m_connectionDelay; // microseconds
	long double m_avgDelay;
	int m_delayCnt;

	bool m_bCanConnect;
	bool m_bDisconnect;

	mutex m_updateAvgLock;
	mutex m_updateDelayLock;

	int m_updateCnt;
};
extern DelayManager gDelayMgr;

class StressTestClient
{
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

	//high_resolution_clock::time_point m_connectStartTime;
	//duration<long long, micro> m_lastConnectTime;

	COORD m_initCursor;
};