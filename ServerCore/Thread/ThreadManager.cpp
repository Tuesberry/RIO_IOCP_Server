#include "ThreadManager.h"

thread_local int ThreadId;
thread_local atomic<int> ThreadCQNum;


/* --------------------------------------------------------
*	Method:		ThreadManager::ThreadManager
*	Summary:	constructor
-------------------------------------------------------- */
ThreadManager::ThreadManager()
	: m_threads()
	, m_threadCnt(0)
	, m_lock()
{
}

/* --------------------------------------------------------
*	Method:		ThreadManager::~ThreadManager
*	Summary:	destructor
-------------------------------------------------------- */
ThreadManager::~ThreadManager()
{
	JoinThreads();
}

/* --------------------------------------------------------
*	Method:		ThreadManager::CreateThread
*	Summary:	create new thread and set thread id
*	Args:		function<void(void)> function
*					function that execute in new thread
-------------------------------------------------------- */
void ThreadManager::CreateThread(function<void(void)> function)
{
	lock_guard<mutex> lock(m_lock);
	
	// create new thread
	m_threads.push_back(thread([=]() 
		{
			// set thread id
			m_threadCnt++;
			ThreadId = m_threadCnt;
			function();
		}));
}

/* --------------------------------------------------------
*	Method:		ThreadManager::JoinThreads
*	Summary:	join threads
-------------------------------------------------------- */
void ThreadManager::JoinThreads()
{
	lock_guard<mutex> lock(m_lock);

	for (thread& thread : m_threads)
	{
		if(thread.joinable())
			thread.join();
	}

	m_threadCnt = 0;
	m_threads.clear();
}
