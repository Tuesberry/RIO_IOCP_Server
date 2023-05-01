#include "RWLock.h"
#include "ThreadManager.h"

/* --------------------------------------------------------
*	Method:		RWLock::RWLock
*	Summary:	constructor
-------------------------------------------------------- */
RWLock::RWLock()
	: m_lockFlag(0)
{
}

/* --------------------------------------------------------
*	Method:		RWLock:ReadLock
*	Summary:	read lock
-------------------------------------------------------- */
void RWLock::ReadLock()
{
	// get read lock
	unsigned __int32 expected;
	while (true)
	{
		for (int i = 0; i < MAX_SPIN_COUNT; i++)
		{
			expected = (m_lockFlag.load() & READ_LOCK_MASK);
			if (m_lockFlag.compare_exchange_weak(expected, expected+1) == true)
			{
				return;
			}
		}

		this_thread::yield();
	}
}

/* --------------------------------------------------------
*	Method:		RWLock:ReadUnlock
*	Summary:	unlock read
-------------------------------------------------------- */
void RWLock::ReadUnlock()
{
	// unlock
	// lock flag check
	if ((m_lockFlag.load() & READ_LOCK_MASK) > 0)
	{
		m_lockFlag.fetch_sub(1);
	}
	if ((m_lockFlag.load() & READ_LOCK_MASK) < 0)
	{
		HandleError("ReadUnlock Error");
	}
}

/* --------------------------------------------------------
*	Method:		RWLock:WriteLock
*	Summary:	lock write
-------------------------------------------------------- */
void RWLock::WriteLock()
{
	// get write lock
	unsigned __int32 expected = EMPTY_FLAG;
	unsigned __int32 desired = (ThreadId << 16) & WRITE_LOCK_MASK;

	while (true)
	{
		for (int i = 0; i < MAX_SPIN_COUNT; i++)
		{
			expected = EMPTY_FLAG;
			if (m_lockFlag.compare_exchange_weak(expected, desired) == true)
			{
				return;
			}
		}

		this_thread::yield();
	}
}

/* --------------------------------------------------------
*	Method:		RWLock:WriteUnlock
*	Summary:	unlock write
-------------------------------------------------------- */
void RWLock::WriteUnlock()
{
	// thread id check
	unsigned __int32 lockThreadId = (m_lockFlag.load() & WRITE_LOCK_MASK) >> 16;
	if (lockThreadId == ThreadId)
	{
		m_lockFlag.store(EMPTY_FLAG);
	}
}
