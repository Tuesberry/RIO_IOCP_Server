#pragma once

#include "Common.h"

#include "Thread/RWLock.h"

/* --------------------------------------------------------
*	class:		LockUnorderedSet
*	Summary:	unordered set using RWLock
-------------------------------------------------------- */
template<typename T>
class LockUnorderedSet
{
public:
	LockUnorderedSet() = default;
	~LockUnorderedSet() = default;

	void Insert(T item)
	{
		WriteLockGuard lock(m_lock);
		m_set.insert(item);
	}

	bool Erase(T item)
	{
		WriteLockGuard lock(m_lock);
		if (m_set.count(item))
		{
			m_set.erase(item);
			return true;
		}
		return false;
	}

	void ReadLock()
	{
		m_lock.ReadLock();
	}

	void ReadUnlock()
	{
		m_lock.ReadUnlock();
	}


public:
	RWLock m_lock;
	unordered_set<T> m_set;
};