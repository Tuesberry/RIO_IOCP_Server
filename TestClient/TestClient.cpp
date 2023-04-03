#pragma once
#include "CoreCommon.h"
#include "pch.h"
#include "Network/IocpClient.h"
#include "ClientSession.h"

int main()
{
	this_thread::sleep_for(1s);

	std::shared_ptr<IocpClient> client = std::make_shared<IocpClient>(
		std::make_shared<IocpCore>(),
		std::make_shared<ClientSession>, // TODO : SessionManager µî
		SockAddress(L"127.0.0.1", 7777),
		5,
		2);

	if (client->StartClient() == false)
	{
		HandleError("StartClient");
		return 0;
	}
	
	client->RunClient();

	client->JoinWorkerThreads();
}