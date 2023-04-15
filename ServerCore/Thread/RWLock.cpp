#include "RWLock.h"
#include "ThreadManager.h"

/* --------------------------------------------------------
*	Method:		RWLock::RWLock
*	Summary:	constructor
-------------------------------------------------------- */
RWLock::RWLock()
	: m_lockFlag(0)
	, m_writeLockCnt(0)
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
		expected = m_lockFlag.load() & READ_LOCK_MASK;
		for (int i = 0; i < MAX_SPIN_COUNT; i++)
		{
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
	// this thread currently using lock?
	unsigned __int32 lockThreadId = (m_lockFlag.load() & WRITE_LOCK_MASK) >> 16;
	if (lockThreadId == ThreadId)
	{
		m_writeLockCnt.fetch_add(1);
		return;
	}

	// get write lock
	unsigned __int32 expected = EMPTY_FLAG;
	unsigned __int32 desired = (ThreadId << 16) & WRITE_LOCK_MASK;
	while (true)
	{
		for (int i = 0; i < MAX_SPIN_COUNT; i++)
		{
			if (m_lockFlag.compare_exchange_weak(expected, desired) == true)
			{
				m_writeLockCnt.fetch_add(1);
				return;
			}
			else
			{
				expected = EMPTY_FLAG;
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
		m_writeLockCnt.fetch_sub(1);
	}

	// unlock
	if (m_writeLockCnt.load() == 0)
	{
		m_lockFlag.store(EMPTY_FLAG);
	}
}
