#include "CoreCommon.h"

void HandleError(const char* cause)
{
    _int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;
}
