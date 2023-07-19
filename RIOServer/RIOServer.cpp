#pragma once

#include "ServerCommon.h"

#include "Thread/ThreadManager.h"

#include "Network/RioServer.h"
#include "Session/RioServerSession.h"

int main()
{
    shared_ptr<RioServer> server = std::make_shared<RioServer>(
        std::make_shared<RioServerSession>,
        SockAddress(L"127.0.0.1", 7777)
        );

    if (!server->InitServer())
    {
        return 0;
    }

    server->RunServer();

    gThreadMgr->JoinThreads();
}