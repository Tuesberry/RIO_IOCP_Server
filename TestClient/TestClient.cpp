#pragma once

#include "pch.h"
#include "CoreCommon.h"

#include "StressTestClient.h"
#include "ClientSession.h"

#define CLIENT_NUM 300

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
	);
	
	stressTestClient.RunServer();
}