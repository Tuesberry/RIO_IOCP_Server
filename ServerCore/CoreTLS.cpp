#include "CoreTLS.h"

thread_local int LThreadId;

thread_local JobQueue* LCurrentJobQueue;
thread_local ULONGLONG LEndTickCount;