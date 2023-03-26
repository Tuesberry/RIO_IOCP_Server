#pragma once

#include <iostream>
using namespace std;

#include <thread>
#include <vector>
#include <set>

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <wchar.h>

#include <mutex>

void HandleError(const char* cause);