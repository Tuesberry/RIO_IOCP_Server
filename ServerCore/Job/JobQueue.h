#pragma once

#include "Common.h"

#include "Utils/LockQueue.h"
#include "Job.h"

/* --------------------------------------------------------
*	class:		JobQueue
*	Summary:	JobQueue to execute job async
-------------------------------------------------------- */
class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	void DoAsync(std::function<void()>&& callback)
	{
		Push(make_shared<Job>(std::move(callback)));
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		Push(make_shared<Job>(owner, memFunc, std::forward<Args>(args)...));
	}

	void Execute();

	bool IsAllocated() { return m_bAllocated.load(); }

	int GetAllocatedThreadId() { return m_threadId; }
	void SetAllocatedThreadId(int threadId) { m_threadId.store(threadId); }

private:
	void Push(shared_ptr<Job>&& job);

	atomic<bool> m_bAllocated = false;
	atomic<int> m_threadId = -1;

protected:
	atomic<int> m_jobCount = 0;
	LockQueue<shared_ptr<Job>> m_jobs;
};