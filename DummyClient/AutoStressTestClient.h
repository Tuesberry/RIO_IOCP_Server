#pragma once

#include "Common.h"

#include "Network/IocpClient.h"
#include "ClientPacketHandler.h"
#include "Thread/RWLock.h"


/* --------------------------------------------------------
*	class:		CheckMaxPointClient
*	Summary:	client for check maximum point of server
-------------------------------------------------------- */

class AutoStressTestClient
{
	enum
	{
		START_NUM = 5990,
		INCREASE_RATE_DEFAULT = 100,
		DELAY_LIMIT = 100, // 100ms
	};

public:
	AutoStressTestClient(shared_ptr<IocpClient> client, int threadCnt = thread::hardware_concurrency());

	AutoStressTestClient() = delete;
	AutoStressTestClient(const AutoStressTestClient& other) = delete;
	AutoStressTestClient(AutoStressTestClient&& other) = delete;
	AutoStressTestClient& operator=(const AutoStressTestClient& other) = delete;
	AutoStressTestClient& operator=(AutoStressTestClient&& other) = delete;
	~AutoStressTestClient();

	void RunStressTestClient();
	void RunToFindMaxConcurrentConn();

private:
	void CreateSenderThreads();
	bool ConnectToServer(int clientNum);
	void ResetSendTime();
	//void DisconnectFromServer(int idx);
	bool SendToServer(int idx);

	void PrintOutput();

private:
	// iocp client
	shared_ptr<IocpClient> m_client;

	// draw output
	COORD m_initCursor;

	// stress test
	bool m_bRunClient;
	bool m_bStopTest;
	bool m_bSendLoginOnly;
	
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