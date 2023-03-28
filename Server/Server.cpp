#pragma once
#include "pch.h"
#include "CoreCommon.h"
#include "Network/IocpServer.h"
#include "ServerSession.h"

int main()
{
    shared_ptr<IocpServer> server = std::make_shared<IocpServer>(
        std::make_shared<IocpCore>(),
        std::make_shared<ServerSession>, 
        SockAddress(L"127.0.0.1", 7777),
        50,
        1);

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
