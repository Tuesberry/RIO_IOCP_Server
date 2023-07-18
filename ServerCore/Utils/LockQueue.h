#pragma once

#include "Common.h"

#include "Thread/RWLock.h"

/* --------------------------------------------------------
*	class:		LockQueue
*	Summary:	queue using RWLock
-------------------------------------------------------- */
template<typename T>
class LockQueue
{
public:
	void Push(T item)
	{
		WriteLockGuard lock(m_rwLock);
		m_queue.push(item);
	}

	T Pop()
	{
		WriteLockGuard lock(m_rwLock);
		if (m_queue.empty())
			return T();

		T ret = m_queue.front();
		m_queue.pop();
		return ret;
	}

	void PopAll(vector<T>& items)
	{
		WriteLockGuard lock(m_rwLock);
		while (m_queue.size())
		{
			T item = m_queue.front();
			m_queue.pop();
			items.push_back(item);
		}
	}

	void Clear()
	{
		WriteLockGuard lock(m_rwLock);
		m_queue = queue<T>();
	}

private:
	RWLock m_rwLock;
	queue<T> m_queue;
};