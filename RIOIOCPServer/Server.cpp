#pragma once

#include "ServerCommon.h"

#include "Thread/ThreadManager.h"

#include "Network/ServerProxy.h"
#include "Session/RioServerSession.h"
#include "Session/IocpServerSession.h"

int main()
{
#ifdef RIO
    shared_ptr<ServerProxy> server = std::make_shared<ServerProxy>(
        ServerType::RIO_SERVER,
        SockAddress("127.0.0.1", 7777),
        std::make_shared<RioServerSession>
    );
    server->Start(true);
#endif
#ifdef IOCP
    shared_ptr<ServerProxy> server = std::make_shared<ServerProxy>(
        ServerType::IOCP_SERVER,
        SockAddress("127.0.0.1", 7777),
        std::make_shared<IocpServerSession>
    );
    server->Start(true);
#endif
    gThreadMgr->JoinThreads();
}