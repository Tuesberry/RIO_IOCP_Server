#pragma once

#include "DummyClientCommon.h"

#include "StressTest/StressTestClient.h"
#include "Session/ClientSession.h"
#include "Thread/ThreadManager.h"
#include "Network/SocketCore.h"
#include "Network/IocpClient.h"

#define START_CLIENT_NUM 2310
#define STRESS_TEST_THREAD_CNT 1

int main()
{
	this_thread::sleep_for(1s);

	shared_ptr<IocpClient> iocpClient = std::make_shared<IocpClient>(
		std::make_shared<IocpCore>(),
		std::make_shared<ClientSession>,
		SockAddress("127.0.0.1", 7777)
	);

	StressTestClient stressTestClient(iocpClient, START_CLIENT_NUM, ETestMode::INCREASE, STRESS_TEST_THREAD_CNT);
	stressTestClient.RunClient();

	gThreadMgr->JoinThreads();
}