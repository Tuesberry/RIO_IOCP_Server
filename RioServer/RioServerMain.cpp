#pragma once

#include "Common.h"

#include "Network/RioServer.h"
#include "RioServerSession.h"

int main()
{
    RioServer server(make_shared<RioServerSession>);

    if (server.InitServer() == false)
    {
        HandleError("InitServer");
        return 0;
    }

    if (server.StartServer() == false)
    {
        HandleError("StartServer");
        return 0;
    }

    gThreadMgr.JoinThreads();
}