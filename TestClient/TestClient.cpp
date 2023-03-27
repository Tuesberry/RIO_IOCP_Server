#include "CoreCommon.h"
#include "pch.h"
#include "Network/IocpClient.h"
#include "Network/IocpSession.h"

char sendBuffer[] = "Hello World";

class ServerSession : public IocpSession
{
public:
	~ServerSession()
	{
		cout << "~ServerSession" << endl;
	}

	virtual void OnConnected() override
	{
		cout << "Connected To Server" << endl;
		Send((BYTE*)sendBuffer, sizeof(sendBuffer));
	}

	virtual int OnRecv(BYTE* buffer, int len) override
	{
		cout << "OnRecv Len = " << len << endl;

		this_thread::sleep_for(1s);

		Send((BYTE*)sendBuffer, sizeof(sendBuffer));
		return len;
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

int main()
{
	this_thread::sleep_for(1s);

	std::shared_ptr<IocpClient> client = std::make_shared<IocpClient>(
		std::make_shared<IocpCore>(),
		std::make_shared<ServerSession>, // TODO : SessionManager µî
		SockAddress(L"127.0.0.1", 7777),
		1,
		1);

	if (client->StartClient() == false)
	{
		HandleError("StartClient");
		return 0;
	}
	
	client->RunClient();

	client->JoinWorkerThreads();
}