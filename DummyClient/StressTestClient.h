#pragma once

#include "Common.h"

#include "Network/IocpClient.h"
#include "ClientPacketHandler.h"
#include "Thread/RWLock.h"

/* --------------------------------------------------------
*	class:		TestSessionManager
*	Summary:	session manager for stress test
-------------------------------------------------------- */

class TestSessionManager
{
public:
	TestSessionManager();
	TestSessionManager(const TestSessionManager& other) = delete;
	TestSessionManager(TestSessionManager&& other) = delete;
	TestSessionManager& operator=(const TestSessionManager& other) = delete;
	TestSessionManager& operator=(TestSessionManager&& other) = delete;
	~TestSessionManager() = default;

	bool IsEmpty() { return m_connectCnt == 0; }

	bool SendLogin(int idx);
	bool SendMove(int idx);
	bool SendPacket(int idx);

	void Disconnect(int idx);

	void AddSession(weak_ptr<ClientSession> session);
	void DeleteSession(int idx);

private:
	RWLock m_rwLock;
	vector<weak_ptr<ClientSession>> m_sessions;
	int m_size;
	int m_connectCnt;
};

extern TestSessionManager gTestSessionMgr;

/* --------------------------------------------------------
*	class:		StressTestClient
*	Summary:	client for stress test
-------------------------------------------------------- */

class StressTestClient
{
	enum : int
	{
		PACKET_SEND_DURATION = 1000,
		STRESS_TEST_TIME_SEC = 600,
	};

public:
	StressTestClient(shared_ptr<IocpClient> client, int clientNum, int threadCnt = thread::hardware_concurrency());

	StressTestClient() = delete;
	StressTestClient(const StressTestClient& other) = delete;
	StressTestClient(StressTestClient&& other) = delete;
	StressTestClient& operator=(const StressTestClient& other) = delete;
	StressTestClient& operator=(StressTestClient&& other) = delete;
	~StressTestClient();

	void RunClient();

private:
	void ConnectToServer();
	void DisconnectFromServer(int idx);
	bool SendToServer(int idx);
	void ResetSendTime();

	void InitOutput();
	void UpdateOutput();
	void TestStopOutput();

	void MoveCursor(int x, int y);

private:
	// iocp client
	shared_ptr<IocpClient> m_client;

	// draw output
	COORD m_initCursor;

	// stress test
	bool m_runClient;
	int m_startTime;

	int m_clientNum;
	int m_threadCnt;
	int m_jobCnt;
	vector<__int64> m_sendTime;
};