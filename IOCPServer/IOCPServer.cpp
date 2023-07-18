#pragma once

#include "IOCPServerCommon.h"

#include "Job/GlobalQueue.h"
#include "Thread/ThreadManager.h"

#include "Network/IocpServer.h"
#include "Session/IocpServerSession.h"

void WorkerJob(shared_ptr<IocpServer> server)
{
    while (true)
    {
        LEndTickCount = ::GetTickCount64() + WORKER_TICK;
        
        server->GetIocpCore()->Dispatch(10);
        
        while (true)
        {
            ULONG now = ::GetTickCount64();
            if (now > LEndTickCount)
            {
                break;
            }
            shared_ptr<JobQueue> jobQueue = gGlobalQueue->Pop();
            if (jobQueue == nullptr)
            {
                break;
            }
            jobQueue->Execute();
        }
    }
}

int main()
{
    shared_ptr<IocpServer> server = std::make_shared<IocpServer>(
        std::make_shared<IocpCore>(),
        std::make_shared<IocpServerSession>,
        SockAddress(L"127.0.0.1", 7777),
        50,
        1);

    if (!server->Start())
    {
        HandleError("ServerStart");
        return 0;
    }
    
    server->RunServer([&server]()
        {
            WorkerJob(server);
        });

    gThreadMgr->JoinThreads();
}
