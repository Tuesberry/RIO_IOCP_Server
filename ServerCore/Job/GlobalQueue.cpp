#include "GlobalQueue.h"

GlobalQueue::GlobalQueue()
{
}

GlobalQueue::~GlobalQueue()
{
}

void GlobalQueue::Push(shared_ptr<JobQueue> jobQueue)
{
	m_jobQueues.Push(jobQueue);
}

shared_ptr<JobQueue> GlobalQueue::Pop()
{
	return m_jobQueues.Pop();
}

void GlobalQueue::PushToList(shared_ptr<JobQueue> jobQueue)
{
	lock_guard<mutex> lock(m_vectorLock);
	m_jobList.push_back(jobQueue);
}

shared_ptr<JobQueue> GlobalQueue::GetRefToJob(int idx)
{
	if (idx < m_jobList.size()) return m_jobList[idx];
	return nullptr;
}
