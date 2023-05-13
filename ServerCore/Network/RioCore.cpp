#include "RioCore.h"
#include "SocketCore.h"
#include "RioEvent.h"
#include "RioSession.h"

/* --------------------------------------------------------
*	Method:		RioCore::RioCore
*	Summary:	constructor
-------------------------------------------------------- */
RioCore::RioCore()
	: m_rioCompletionQueue()
	, m_results()
	, m_sessionLock()
	, m_sessions()
	, m_sessionCnt(0)
	, m_bInit(false)
{
}

/* --------------------------------------------------------
*	Method:		RioCore::InitRioCore
*	Summary:	Create and initialize completion queue
-------------------------------------------------------- */
bool RioCore::InitRioCore()
{
	// create completion queue
	m_rioCompletionQueue = SocketCore::RIO.RIOCreateCompletionQueue(MAX_CQ_SIZE, 0);
	if (m_rioCompletionQueue == RIO_INVALID_CQ)
	{
		HandleError("RIOCreateCompletionQueue");
		return false;
	}
	
	return true;
}

/* --------------------------------------------------------
*	Method:		RioCore::Dispatch
*	Summary:	dequeue completion queue
-------------------------------------------------------- */
bool RioCore::Dispatch()
{
	// reset result
	memset(m_results, 0, sizeof(m_results));

	// dequeue completion
	ULONG numResults = SocketCore::RIO.RIODequeueCompletion(m_rioCompletionQueue, m_results, MAX_RIO_RESULT);

	// check numResults
	if (numResults == 0)
	{
		// this_thread::sleep_for(1ms);
		this_thread::yield();
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
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		RioCore::DeferredSend
*	Summary:	dequeue data from the message queue
*				and transmit to client
-------------------------------------------------------- */
void RioCore::DeferredSend()
{
	lock_guard<mutex> lock(m_sessionLock);
	for (auto sIter = m_sessions.begin(); sIter != m_sessions.end(); sIter++)
	{
		while (true)
		{
			if ((*sIter)->SendDeferred() == false)
				break;
			//cout << "SendComplete" << endl;
		}
	}
}

/* --------------------------------------------------------
*	Method:		RioCore::AddSession
*	Summary:	Add session to rio core
-------------------------------------------------------- */
void RioCore::AddSession(shared_ptr<RioSession> session)
{
	lock_guard<mutex> lock(m_sessionLock);
	m_sessions.insert(session);
	m_sessionCnt++;
}

/* --------------------------------------------------------
*	Method:		RioCore::ReleaseSession
*	Summary:	Release session in rio core
-------------------------------------------------------- */
void RioCore::ReleaseSession(shared_ptr<RioSession> session)
{
	lock_guard<mutex> lock(m_sessionLock);
	m_sessions.erase(session);
	m_sessionCnt--;
}
