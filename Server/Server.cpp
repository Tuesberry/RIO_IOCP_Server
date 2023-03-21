#include "pch.h"
#include "CoreCommon.h"
#include "Network/IocpService.h"
#include "Network/IocpSession.h"

class GameSession : public IocpSession
{
public:
    ~GameSession()
    {
        cout << "~GameSession" << endl;
    }

    virtual int OnRecv(BYTE* buffer, int len) override
    {
        // Echo
        cout << "OnRecv Len = " << len << endl;
        Send(buffer, len);
        return len;
    }

    virtual void OnSend(int len) override
    {
        cout << "OnSend Len = " << len << endl;
    }
};

int main()
{
    shared_ptr<IocpServerService> service = std::make_shared<IocpServerService>(
        SockAddress("127.0.0.1", 7777),
        std::make_shared<IocpCore>(),
        std::make_shared<GameSession>, // TODO : SessionManager µî
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
