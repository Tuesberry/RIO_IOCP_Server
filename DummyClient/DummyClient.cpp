#pragma once

#include "DummyClientCommon.h"

#include "StressTest/StressTestClient.h"
#include "Session/ClientSession.h"
#include "Thread/ThreadManager.h"
#include "Network/SocketCore.h"

#define START_CLIENT_NUM 1000
#define STRESS_TEST_THREAD_CNT 1

int main()
{
	this_thread::sleep_for(1s);

	shared_ptr<IocpClient> iocpClient = std::make_shared<IocpClient>(
		std::make_shared<IocpCore>(),
		std::make_shared<ClientSession>,
		SockAddress(L"127.0.0.1", 7777),
		500,
		1);

	StressTestClient stressTestClient(iocpClient, START_CLIENT_NUM, ETestMode::NORMAL, STRESS_TEST_THREAD_CNT);
	stressTestClient.RunClient();

	gThreadMgr->JoinThreads();
}