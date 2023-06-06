#pragma once

#include "Common.h"

#include "Network/IocpClient.h"
#include "ClientPacketHandler.h"
#include "Thread/RWLock.h"


/* --------------------------------------------------------
*	class:		CheckMaxConnectionClient
*	Summary:	client for check max point
-------------------------------------------------------- */

class CheckMaxConnectionClient
{
	enum
	{
		START_CONN_CNT = 4000,
		INCREASE_RATE_DEFAULT = 100,
		DELAY_LIMIT = 100, // 100ms
	};

public:
	CheckMaxConnectionClient(shared_ptr<IocpClient> client, int threadCnt = thread::hardware_concurrency());

	CheckMaxConnectionClient() = delete;
	CheckMaxConnectionClient(const CheckMaxConnectionClient& other) = delete;
	CheckMaxConnectionClient(CheckMaxConnectionClient&& other) = delete;
	CheckMaxConnectionClient& operator=(const CheckMaxConnectionClient& other) = delete;
	CheckMaxConnectionClient& operator=(CheckMaxConnectionClient&& other) = delete;
	~CheckMaxConnectionClient();

	void RunClient();

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