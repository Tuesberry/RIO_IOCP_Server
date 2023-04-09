#pragma once
#include "CoreCommon.h"
#include "Network/IocpClient.h"

class StressTestClient
{
	enum
	{
		PACKET_SEND_DURATION = 1000,
	};

public:
	StressTestClient() = delete;
	StressTestClient(shared_ptr<IocpClient> client, int clientNum);

	StressTestClient(const StressTestClient& other) = delete;
	StressTestClient(StressTestClient&& other) = delete;
	StressTestClient& operator=(const StressTestClient& other) = delete;
	StressTestClient& operator=(StressTestClient&& other) = delete;
	~StressTestClient();

	void RunServer();

private:
	void UpdateSessions(int idx);
	void SendPacketToServer(int idx, int duration);
	void ConnectToServer(int idx, int duration);

	void InitOutput();
	void UpdateOutput();

	void MoveCursor(int x, int y);

private:
	shared_ptr<IocpClient> m_client;

	COORD m_initCursor;

	// stress test
	int m_clientNum;
	int m_coreCnt;
	int m_jobCnt;
	vector<thread> m_threads;
	bool m_bConnect;
};