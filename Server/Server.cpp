#include "pch.h"
#include "CoreCommon.h"
#include "Network/IocpServer.h"
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
    shared_ptr<IocpServer> server = std::make_shared<IocpServer>(
        std::make_shared<IocpCore>(),
        std::make_shared<GameSession>, 
        SockAddress(L"127.0.0.1", 7777),
        10,
        2);

    if (server->StartServer() == false)
    {
        HandleError("StartServer");
        return 0;
    }
    
    server->RunServer();
    /*
    while (true)
    {
        string inputCmd;
        getline(std::cin, inputCmd);

        if (inputCmd == "quit")
        {
            break;
        }
    }
    */
    server->JoinWorkerThreads();

    return 0;
}
