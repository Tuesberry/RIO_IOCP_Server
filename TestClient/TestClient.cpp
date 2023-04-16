#pragma once

#include "Common.h"

#include "StressTestClient.h"
#include "ClientSession.h"

#define CLIENT_NUM 300
#define STRESS_TEST_THREAD_CNT 2

int main()
{
	this_thread::sleep_for(1s);

	StressTestClient stressTestClient(std::make_shared<IocpClient>(
		std::make_shared<IocpCore>(),
		std::make_shared<ClientSession>, 
		SockAddress(L"127.0.0.1", 7777),
		500,
		1)
		,CLIENT_NUM
		,STRESS_TEST_THREAD_CNT
	);
	
	stressTestClient.RunClient();
}