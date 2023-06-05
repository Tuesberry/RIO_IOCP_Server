#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")
#else
#pragma comment(lib, "Release\\ServerCore.lib")
#endif // _DEBUG

#include "CoreCommon.h"

enum TestConfig
{
	PACKET_SEND_DURATION = 1000,
	STRESS_TEST_TIME_SEC = 5,
};