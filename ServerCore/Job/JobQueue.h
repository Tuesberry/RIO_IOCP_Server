#pragma once

#include "Common.h"

#include "Thread/RWLock.h"
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

private:
	void Push(shared_ptr<Job>&& job);
	void Execute();

protected:
	atomic<int> m_jobCount = 0;
	LockQueue<shared_ptr<Job>> m_jobs;
};