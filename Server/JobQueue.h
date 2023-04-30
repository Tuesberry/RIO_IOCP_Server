#pragma once

#include "Common.h"

#include "Thread/RWLock.h"
#include "Job.h"

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

private:
	RWLock m_rwLock;
	queue<T> m_queue;
};

/* --------------------------------------------------------
*	class:		JobQueue
*	Summary:	JobQueue to execute job async
-------------------------------------------------------- */
class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	void DoAsync(std::function<void()>&& callback)
	{
		m_jobs.Push(make_shared<Job>(std::move(callback)));
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		m_jobs.Push(make_shared<Job>(owner, memFunc, std::forward<Args>(args)...));
	}

	void Execute()
	{
		shared_ptr<Job> job = m_jobs.Pop();
		if (job)
		{
			job->Execute();
		}
	}
private:
	LockQueue<shared_ptr<Job>> m_jobs;
};