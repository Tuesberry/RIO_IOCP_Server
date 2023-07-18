#include "CoreGlobal.h"
#include "Thread/ThreadManager.h"
#include "Job/GlobalQueue.h"
#include "Network/SocketCore.h"

ThreadManager* gThreadMgr = nullptr;
GlobalQueue* gGlobalQueue = nullptr;

class CoreGlobal
{
public:
    CoreGlobal()
    {
        SocketCore::Init();
        gThreadMgr = new ThreadManager();
        gGlobalQueue = new GlobalQueue();
    }

    ~CoreGlobal()
    {
        delete gThreadMgr;
        delete gGlobalQueue;
        SocketCore::Clear();
    }
} GCoreGlobal;