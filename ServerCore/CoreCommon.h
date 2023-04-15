#pragma once

#include <iostream>
using namespace std;

#include <random>
#include <chrono>
using namespace chrono;

#include <vector>
#include <set>
#include <queue>
#include <map>
#include <unordered_set>

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <wchar.h>
#include <string>
#include <functional>

#include <thread>
#include <mutex>

#include "Thread/ThreadManager.h"

#define BUFSIZE 1000
#define len(arr) static_cast<int>(sizeof(arr)/sizeof(arr[0]))

void HandleError(const char* cause);

extern class ThreadManager gThreadMgr;