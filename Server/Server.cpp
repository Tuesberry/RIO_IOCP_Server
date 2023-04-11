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
        50,
        2));

    stressTestServer.RunServer();

    return 0;
}
