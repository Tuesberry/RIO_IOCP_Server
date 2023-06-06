#pragma once

#include "Common.h"

#include "StressTestClient.h"
#include "AutoStressTestClient.h"

#include "ClientSession.h"

//debug
#include "Network/SocketCore.h"

#define CLIENT_NUM 3200
#define STRESS_TEST_THREAD_CNT 2

#include "Network/RingBuffer.h"

int main()
{
	// set logger
	// Logger::SetFileLog("Clientlog.txt");
	
	this_thread::sleep_for(1s);

	shared_ptr<IocpClient> iocpClient = std::make_shared<IocpClient>(
		std::make_shared<IocpCore>(),
		std::make_shared<ClientSession>,
		SockAddress(L"127.0.0.1", 7777),
		500,
		1);

	//StressTestClient stressTestClient(iocpClient, CLIENT_NUM, STRESS_TEST_THREAD_CNT);
	//stressTestClient.RunClient();

	AutoStressTestClient checkMaxClient(iocpClient, STRESS_TEST_THREAD_CNT);
	checkMaxClient.RunStressTestClient();

	//checkMaxClient.RunToFindMaxConcurrentConn();

	gThreadMgr.JoinThreads();
}