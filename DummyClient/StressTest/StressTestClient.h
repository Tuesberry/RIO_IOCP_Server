#pragma once

#include "Common.h"

#include "../Session/PacketHandler.h"
#include "Thread/RWLock.h"

/* --------------------------------------------------------
*	class:		ETestMode
*	Summary:	Used to select test mode
-------------------------------------------------------- */

enum class ETestMode
{
	NORMAL,
	INCREASE
};

class IocpClient;

/* --------------------------------------------------------
*	class:		StressTestClient
*	Summary:	client for stress test
-------------------------------------------------------- */
class StressTestClient
{
	enum
	{
		START_NUM = 5990,
		INCREASE_RATE_DEFAULT = 20,
		DELAY_LIMIT = 100, // 100ms
		PACKET_SEND_DURATION = 1000,
		STRESS_TEST_TIME_SEC = 6000,
		STRESS_TEST_DELTA_TIME_SEC = 60,
	};

public:
	StressTestClient(shared_ptr<IocpClient> client, int clientNum, ETestMode testMode = ETestMode::NORMAL, int threadCnt = thread::hardware_concurrency());

	StressTestClient() = delete;
	StressTestClient(const StressTestClient& other) = delete;
	StressTestClient(StressTestClient&& other) = delete;
	StressTestClient& operator=(const StressTestClient& other) = delete;
	StressTestClient& operator=(StressTestClient&& other) = delete;
	~StressTestClient();

	void RunClient();

private:
	// server communication
	bool ConnectToServer(int clientNum);
	void DisconnectFromServer(int idx);
	bool SendToServer(int idx);

	// thread
	void CreateSenderThreads();
	
	// client
	bool RunDummyClient();

	// stress test
	void StartNormalMode();
	void StartIncreaseMode();

	// send time
	void ResetSendTime();

	// output
	void InitOutput();
	void UpdateOutput();
	void TestStopOutput();
	void PrintOutput();
	void MoveCursor(int x, int y);

private:
	// iocp client
	shared_ptr<IocpClient> m_client;

	// draw output
	COORD m_initCursor;

	// stress test
	bool m_bRunClient;
	bool m_bDisconnectComplete;
	bool m_bStopTest;

	// test mode
	ETestMode m_testMode;

	// test start time
	int m_startTime;

	// test client num
	int m_clientNum;
	int m_increaseRate;

	// thread
	int m_threadCnt;
	int m_jobCnt;
	vector<__int64> m_sendTime;
};