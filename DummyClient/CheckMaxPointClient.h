#pragma once

#include "Common.h"

#include "Network/IocpClient.h"
#include "ClientPacketHandler.h"
#include "Thread/RWLock.h"


/* --------------------------------------------------------
*	class:		CheckMaxPointClient
*	Summary:	client for check maximum point of server
-------------------------------------------------------- */

class CheckMaxPointClient
{
public:
	CheckMaxPointClient(shared_ptr<IocpClient> client, int threadCnt = thread::hardware_concurrency());

	CheckMaxPointClient() = delete;
	CheckMaxPointClient(const CheckMaxPointClient& other) = delete;
	CheckMaxPointClient(CheckMaxPointClient&& other) = delete;
	CheckMaxPointClient& operator=(const CheckMaxPointClient& other) = delete;
	CheckMaxPointClient& operator=(CheckMaxPointClient&& other) = delete;
	~CheckMaxPointClient();

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