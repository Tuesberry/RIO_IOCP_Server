#pragma once

#include "Common.h"

#if IOCP
#include "Network/IocpServer.h"
#include "IocpServerSession.h"
#else RIO
#include "Network/RioServer.h"
#include "RioServerSession.h"
#endif

#include "StressTestServer.h"

int main()
{
    // set logger
    // Logger::SetFileLog("Serverlog.txt");

    // server init
#if IOCP
    shared_ptr<IocpServer> server = std::make_shared<IocpServer>(
        std::make_shared<IocpCore>(),
        std::make_shared<IocpServerSession>,
        SockAddress(L"127.0.0.1", 7777),
        50,
        2);
#else RIO
    shared_ptr<RioServer> server = std::make_shared<RioServer>(
        std::make_shared<RioServerSession>,
        SockAddress(L"127.0.0.1", 7777)
        );
#endif

    // create server
    StressTestServer stressTestServer(server);

    // run server
    stressTestServer.RunServer();

    return 0;
}
