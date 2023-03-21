#include "CoreCommon.h"
#include "pch.h"
#include "Network/IocpService.h"
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

	std::shared_ptr<IocpClientService> service = std::make_shared<IocpClientService>(
		SockAddress("127.0.0.1", 7777),
		std::make_shared<IocpCore>(),
		std::make_shared<ServerSession>, // TODO : SessionManager µî
		10);

	service->Start();

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	vector<thread> threads;
	for (int i = 0; i < si.dwNumberOfProcessors; i++)
	{
		threads.push_back(thread([=]()
			{
				while (true)
				{
					service->GetIocpCore()->ExecuteTask();
				}
			}));
	}

	// thread join
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}

}