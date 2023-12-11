#pragma once
#include "Common.h"

#include "../Utils/LockQueue.h"
#include "JobQueue.h"

class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();

	void Push(shared_ptr<JobQueue> jobQueue);
	shared_ptr<JobQueue> Pop();

	void PushToList(shared_ptr<JobQueue> jobQueue);
	shared_ptr<JobQueue> GetRefToJob(int idx);

	int GetSize() { return m_jobQueues.GetSize(); }

private:
	LockQueue<shared_ptr<JobQueue>> m_jobQueues;

	mutex m_vectorLock;
	vector<shared_ptr<JobQueue>> m_jobList;
};