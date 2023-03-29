#pragma once
#include "pch.h"
#include "CoreCommon.h"
#include "Network/IocpServer.h"
#include "ServerSession.h"
#include "StressTestServer.h"

int main()
{
    StressTestServer stressTestServer(std::make_shared<IocpServer>(
        std::make_shared<IocpCore>(),
        std::make_shared<ServerSession>,
        SockAddress(L"127.0.0.1", 7777),
        20,
        1));

    stressTestServer.RunServer();

 /*   shared_ptr<IocpServer> server = std::make_shared<IocpServer>(
        std::make_shared<IocpCore>(),
        std::make_shared<ServerSession>, 
        SockAddress(L"127.0.0.1", 7777),
        20,
        1);

    if (server->StartServer() == false)
    {
        HandleError("StartServer");
        return 0;
    }
    
    server->RunServer();

    server->JoinWorkerThreads();*/

    return 0;
}
