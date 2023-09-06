#pragma once

#include "Common.h"

extern thread_local int LThreadId;

extern thread_local class JobQueue* LCurrentJobQueue;
extern thread_local unsigned long long LEndTickCount;