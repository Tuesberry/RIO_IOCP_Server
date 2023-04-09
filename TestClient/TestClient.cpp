#pragma once
#include "CoreCommon.h"
#include "pch.h"
#include "StressTestClient.h"
#include "ClientSession.h"

int main()
{
	this_thread::sleep_for(1s);
	int ClientNum = 100;

	StressTestClient stressTestClient(std::make_shared<IocpClient>(
		std::make_shared<IocpCore>(),
		std::make_shared<ClientSession>, // TODO : SessionManager µî
		SockAddress(L"127.0.0.1", 7777),
		500,
		2)
		,ClientNum
	);
	
	stressTestClient.RunServer();
}