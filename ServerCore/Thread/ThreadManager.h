#pragma once

#include "Common.h"

/* --------------------------------------------------------
*	class:		ThreadManager
*	Summary:	manage threads
-------------------------------------------------------- */

class ThreadManager
{
public:
	ThreadManager();
	ThreadManager(const ThreadManager& other) = delete;
	ThreadManager(ThreadManager&& other) = delete;
	ThreadManager& operator=(const ThreadManager& other) = delete;
	ThreadManager& operator=(ThreadManager&& other) = delete;
	~ThreadManager();

	void CreateThread(function<void(void)> function);
	void JoinThreads();

private:
	vector<thread> m_threads;
	int m_threadCnt;
	mutex m_lock;
};