#include "RWLock.h"
#include "ThreadManager.h"

/* --------------------------------------------------------
*	Method:		RWLock::RWLock
*	Summary:	constructor
-------------------------------------------------------- */
RWLock::RWLock()
	: m_lockFlag(0)
	, m_writeCnt(0)
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
	if ((m_lockFlag.fetch_sub(1) & READ_LOCK_MASK) == 0)
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
	// if same thread => correct!
	const unsigned __int32 lockThreadId = (m_lockFlag.load() & WRITE_LOCK_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		m_writeCnt++;
		return;
	}

	// get write lock
	unsigned __int32 expected = EMPTY_FLAG;
	unsigned __int32 desired = (LThreadId << 16) & WRITE_LOCK_MASK;

	while (true)
	{
		for (int i = 0; i < MAX_SPIN_COUNT; i++)
		{
			expected = EMPTY_FLAG;
			if (m_lockFlag.compare_exchange_weak(expected, desired) == true)
			{
				m_writeCnt++;
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
	if (lockThreadId == LThreadId)
	{
		const unsigned __int32 lockCnt = --m_writeCnt;
		if (lockCnt == 0)
		{
			m_lockFlag.store(EMPTY_FLAG);
		}
	}
}
