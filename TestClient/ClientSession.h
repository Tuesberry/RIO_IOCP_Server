#pragma once
#include "CoreCommon.h"
#include "pch.h"
#include "Network/IocpSession.h"
#include "Utils/ConsoleOutputManager.h"
char sendData[] = "Hello World";

class ClientSession : public IocpSession
{
public:
	~ClientSession()
	{
		cout << "~ClientSession" << endl;
	}

	virtual void OnConnected() override
	{
		GCoutMgr << "Connected To Server";
		int dataLen = sizeof(sendData) / sizeof(BYTE);
		shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(dataLen);
		::memcpy(sendBuffer->GetData(), &sendData, sizeof(sendData));
		sendBuffer->OnWrite(dataLen);
		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int len) override
	{
		cout << "OnRecv Len = " << len << endl;
		cout << "OnRecv Data = " << buffer << endl;

		this_thread::sleep_for(1s);

		int dataLen = sizeof(sendData) / sizeof(BYTE);
		shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(dataLen);
		::memcpy(sendBuffer->GetData(), &sendData, sizeof(sendData));
		sendBuffer->OnWrite(dataLen);
		Send(sendBuffer);
	}

	virtual void OnSend(int len) override
	{
		cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		cout << "Disconnected" << endl;
	}
};