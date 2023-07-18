#include "JobQueue.h"
#include "GlobalQueue.h"

void JobQueue::Push(shared_ptr<Job>&& job)
{
	const int prevCount = m_jobCount.fetch_add(1);
	m_jobs.Push(job);

	if (prevCount == 0)
	{
		gGlobalQueue->Push(shared_from_this());
	}
}

void JobQueue::Execute()
{
	LCurrentJobQueue = this;

	while (true)
	{
		vector<shared_ptr<Job>> jobs;
		m_jobs.PopAll(jobs);

		const int jobCount = jobs.size();
		for (int i = 0; i < jobCount; i++)
		{
			jobs[i]->Execute();
		}

		if (m_jobCount.fetch_sub(jobCount) == jobCount)
		{
			LCurrentJobQueue = nullptr;
			return;
		}

		const ULONGLONG now = ::GetTickCount64();
		if (now >= LEndTickCount)
		{
			LCurrentJobQueue = nullptr;
			gGlobalQueue->Push(shared_from_this());
			break;
		}
	}
}
