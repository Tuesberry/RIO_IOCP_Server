#include "Common.h"
#include "CoreCommon.h"
#include "Network/SocketCore.h"
#include "Utils/ConsoleOutputManager.h"

ConsoleOutputManager GCoutMgr;

void HandleError(const char* cause)
{
    _int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;
}

class CoreGlobal
{
public:
    CoreGlobal()
    {
        SocketCore::Init();
    }

    ~CoreGlobal()
    {
        SocketCore::Clear();
    }
} GCoreGlobal;