#pragma once

#include <iostream>
using namespace std;

#include <random>
#include <chrono>
#include <time.h>
using namespace chrono;

#include <algorithm>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <unordered_set>
#include <list>

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <wchar.h>
#include <string>
#include <functional>

#include <thread>
#include <mutex>
#include <atomic>
#include <malloc.h>

#include "CoreTLS.h"
#include "CoreGlobal.h"

void HandleError(const char* cause);
