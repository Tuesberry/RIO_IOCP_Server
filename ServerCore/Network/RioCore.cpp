#include "RioCore.h"
#include "SocketCore.h"
#include "RioEvent.h"
#include "RioSession.h"

RioCore::RioCore()
	: m_rioCompletionQueue()
	, m_results()
	, m_sessionLock()
	, m_sessions()
	, m_sessionCnt(0)
	, m_bInit(false)
{
}

bool RioCore::InitRioCore()
{
	// completion queue
	m_rioCompletionQueue = SocketCore::RIO.RIOCreateCompletionQueue(MAX_CQ_SIZE, 0);
	if (m_rioCompletionQueue == RIO_INVALID_CQ)
		return false;
	
	return true;
}

bool RioCore::Dispatch()
{
	// reset result
	memset(m_results, 0, sizeof(m_results));

	// dequeue completion
	ULONG numResults = SocketCore::RIO.RIODequeueCompletion(m_rioCompletionQueue, m_results, MAX_RIO_RESULT);

	// check numResults
	if (numResults == 0)
	{
		this_thread::sleep_for(1s);
		return true;
	}
	else if (numResults == RIO_CORRUPT_CQ)
	{
		HandleError("RIO_CORRUPT_CQ");
		return false;
	}

	// handle results
	for (ULONG i = 0; i < numResults; i++)
	{
		ULONG bytesTransferred = m_results[i].BytesTransferred;

		RioEvent* rioEvent = reinterpret_cast<RioEvent*>(m_results[i].RequestContext);
		shared_ptr<RioSession> session = rioEvent->m_owner;

		if (bytesTransferred == 0)
		{
			session->Disconnect();
		}
		else
		{
			session->Dispatch(rioEvent, bytesTransferred);
		}

		// release event?
	}

	return true;
}

void RioCore::AddSession(shared_ptr<RioSession> session)
{
	lock_guard<mutex> lock(m_sessionLock);
	m_sessions.insert(session);
	m_sessionCnt++;
}

void RioCore::ReleaseSession(shared_ptr<RioSession> session)
{
	lock_guard<mutex> lock(m_sessionLock);
	m_sessions.erase(session);
	m_sessionCnt--;
}
