#pragma once

#include "Common.h"

#include "Thread/RWLock.h"

/* --------------------------------------------------------
*	class:		LockUnorderedSet
*	Summary:	unordered set using RWLock
-------------------------------------------------------- */
template<typename Key, typename Value>
class LockMap
{
public:
	LockMap() = default;
	~LockMap() = default;

	void Insert(Key key, Value value)
	{
		WriteLockGuard lock(m_lock);
		m_map.insert({key,value});
	}

	bool Erase(Key item)
	{
		WriteLockGuard lock(m_lock);
		if (m_map.count(item))
		{
			m_map.erase(item);
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
	std::map<Key, Value> m_map;
};