#pragma once

#include "Common.h"

#include "StressTestClient.h"
#include "ClientSession.h"

//debug
#include "Network/SocketCore.h"

#define CLIENT_NUM 100
#define STRESS_TEST_THREAD_CNT 2

int main()
{
	/*
	this_thread::sleep_for(1s);

	StressTestClient stressTestClient(std::make_shared<IocpClient>(
		std::make_shared<IocpCore>(),
		std::make_shared<ClientSession>,
		SockAddress(L"127.0.0.1", 7777),
		500,
		1)
		, CLIENT_NUM
		, STRESS_TEST_THREAD_CNT
	);

	stressTestClient.RunClient();
	*/
	this_thread::sleep_for(1s);

	char buf[100] = "Hello World!";

	SOCKET clientSocket = SocketCore::Socket();
	
	SockAddress serverAddress(L"127.0.0.1", 8000);
	if (SocketCore::Connect(clientSocket, serverAddress) == false)
	{
		return 0;
	}

	cout << "Connect To Server" << endl;

	while (true)
	{
		int result = SocketCore::Send(clientSocket, buf, sizeof(buf));
		if (result == SOCKET_ERROR)
		{
			HandleError("Send");
		}

		cout << "Send Data! Len = " << sizeof(buf) << endl;

		this_thread::sleep_for(1s);
	}
}