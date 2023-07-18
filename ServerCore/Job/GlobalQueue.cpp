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
