#include "RioCore.h"
#include "SocketCore.h"
#include "RioEvent.h"
#include "RioSession.h"

/* --------------------------------------------------------
*	Method:		RioCore::RioCore
*	Summary:	constructor
-------------------------------------------------------- */
RioCore::RioCore()
	: m_sessionLock()
	, m_sessions()
	, m_sessionCnt(0)
	, m_bInit(false)
#if RIOIOCP
	, m_rioCQEvent()
#endif
#if SEPCQ
	, m_sendCQ()
	, m_recvCQ()
	, m_sendResults()
	, m_recvResults()
#else
	, m_rioCompletionQueue()
	, m_results()
#endif
{
}

/* --------------------------------------------------------
*	Method:		RioCore::InitRioCore
*	Summary:	Create and initialize completion queue
-------------------------------------------------------- */
bool RioCore::InitRioCore(HANDLE iocpHandle)
{	
#if (RIOIOCP && SEPCQ)
	m_rioCQEvent.m_ownerCore = shared_from_this();

	RIO_NOTIFICATION_COMPLETION completionType;

	completionType.Type = RIO_IOCP_COMPLETION;
	completionType.Iocp.IocpHandle = iocpHandle;
	completionType.Iocp.CompletionKey = (void*)1;
	completionType.Iocp.Overlapped = &m_rioCQEvent;

	// create completion queue
	m_recvCQ = SocketCore::RIO.RIOCreateCompletionQueue(MAX_CQ_SIZE, &completionType);
	if (m_recvCQ == RIO_INVALID_CQ)
	{
		HandleError("RIOCreateCompletionQueue");
		return false;
	}

	m_sendCQ = SocketCore::RIO.RIOCreateCompletionQueue(MAX_CQ_SIZE, NULL);
	if (m_sendCQ == RIO_INVALID_CQ)
	{
		HandleError("RIOCreateCompletionQueue : sendQueue");
		return false;
	}
#elif RIOIOCP
	m_rioCQEvent.m_ownerCore = shared_from_this();

	RIO_NOTIFICATION_COMPLETION completionType;

	completionType.Type = RIO_IOCP_COMPLETION;
	completionType.Iocp.IocpHandle = iocpHandle;
	completionType.Iocp.CompletionKey = (void*)1;
	completionType.Iocp.Overlapped = &m_rioCQEvent;

	// create completion queue
	m_rioCompletionQueue = SocketCore::RIO.RIOCreateCompletionQueue(MAX_CQ_SIZE, &completionType);
	if (m_rioCompletionQueue == RIO_INVALID_CQ)
	{
		HandleError("RIOCreateCompletionQueue");
		return false;
	}
#endif
	return true;
}

bool RioCore::InitRioCore()
{
#if SEPCQ
	m_recvCQ = SocketCore::RIO.RIOCreateCompletionQueue(MAX_CQ_SIZE, NULL);
	if (m_recvCQ == RIO_INVALID_CQ)
	{
		HandleError("RIOCreateCompletionQueue : recvQueue");
		return false;
	}

	m_sendCQ = SocketCore::RIO.RIOCreateCompletionQueue(MAX_CQ_SIZE, NULL);
	if (m_sendCQ == RIO_INVALID_CQ)
	{
		HandleError("RIOCreateCompletionQueue : sendQueue");
		return false;
	}
#else
	m_rioCompletionQueue = SocketCore::RIO.RIOCreateCompletionQueue(MAX_CQ_SIZE, NULL);
	if (m_rioCompletionQueue == RIO_INVALID_CQ)
	{
		HandleError("RIOCreateCompletionQueue");
		return false;
	}
#endif
	return true;
}

#if SEPCQ
/* --------------------------------------------------------
*	Method:		RioCoreSepCQ::DispatchSend
*	Summary:	dequeue send completion queue
-------------------------------------------------------- */
bool RioCore::DispatchSend()
{
	// reset result
	memset(m_sendResults, 0, sizeof(m_sendResults));

	// dequeue completion
	ULONG numResults = SocketCore::RIO.RIODequeueCompletion(m_sendCQ, m_sendResults, MAX_RIO_RESULT);

	if (numResults == 0)
	{
		this_thread::yield();
		return false;
	}
	else if (numResults == RIO_CORRUPT_CQ)
	{
		HandleError("RIO_CORRUPT_CQ");
		return false;
	}

	// handle results
	for (ULONG i = 0; i < numResults; i++)
	{
		ULONG bytesTransferred = m_sendResults[i].BytesTransferred;

		RioEvent* rioEvent = reinterpret_cast<RioEvent*>(m_sendResults[i].RequestContext);
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
*	Method:		RioCoreSepCQ::DispatchRecv
*	Summary:	dequeue recv completion queue
-------------------------------------------------------- */
bool RioCore::DispatchRecv()
{
	// reset result
	memset(m_recvResults, 0, sizeof(m_recvResults));

	// dequeue completion
	ULONG numResults = SocketCore::RIO.RIODequeueCompletion(m_recvCQ, m_recvResults, MAX_RIO_RESULT);

#if RIO_IOCP
	// check numResults
	if (numResults == 0 || numResults == RIO_CORRUPT_CQ)
	{
		HandleError("RIO_CORRUPT_CQ");
		return false;
	}
#else
	if (numResults == 0)
	{
		this_thread::yield();
		return false;
	}
	else if (numResults == RIO_CORRUPT_CQ)
	{
		HandleError("RIO_CORRUPT_CQ");
		return false;
	}
#endif

	// handle results
	for (ULONG i = 0; i < numResults; i++)
	{
		ULONG bytesTransferred = m_recvResults[i].BytesTransferred;

		RioEvent* rioEvent = reinterpret_cast<RioEvent*>(m_recvResults[i].RequestContext);
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

#if RIOIOCP
	// notify
	return SetRioNotify();
#endif
	return true;
}
#endif

/* --------------------------------------------------------
*	Method:		RioCore::Dispatch
*	Summary:	dequeue completion queue
-------------------------------------------------------- */
#if !SEPCQ
bool RioCore::Dispatch()
{
	// reset result
	memset(m_results, 0, sizeof(m_results));

	// dequeue completion
	ULONG numResults = SocketCore::RIO.RIODequeueCompletion(m_rioCompletionQueue, m_results, MAX_RIO_RESULT);

#if RIO_IOCP
	// check numResults
	if (numResults == 0 || numResults == RIO_CORRUPT_CQ)
	{
		HandleError("RIO_CORRUPT_CQ");
		return false;
	}
#else
	if (numResults == 0)
	{
		this_thread::yield();
		return false;
	}
	else if (numResults == RIO_CORRUPT_CQ)
	{
		HandleError("RIO_CORRUPT_CQ");
		return false;
	}
#endif

	LThreadCQNum.store(numResults);

	// handle results
	for (ULONG i = 0; i < numResults; i++)
	{
		ULONG bytesTransferred = m_results[i].BytesTransferred;
		
		RioEvent* rioEvent = reinterpret_cast<RioEvent*>(m_results[i].RequestContext);
		shared_ptr<RioSession> session = rioEvent->m_owner;
		
		if (session == nullptr)
		{
			cout << "session null" << endl;
			continue;
		}

		session->Dispatch(rioEvent, bytesTransferred);
	}

#if RIOIOCP
	// notify
	return SetRioNotify();
#endif
}
#endif
/* --------------------------------------------------------
*	Method:		RioCore::SetRioNotify
*	Summary:	set completion queue notify
-------------------------------------------------------- */
#if RIOIOCP
bool RioCore::SetRioNotify()
{
#if SEPCQ
	int notifyResult = SocketCore::RIO.RIONotify(m_recvCQ);
#else
	int notifyResult = SocketCore::RIO.RIONotify(m_rioCompletionQueue);
#endif
	if (notifyResult != ERROR_SUCCESS)
	{
		HandleError("SetRioNotify");
		return false;
	}

	return true;
}
#endif
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
#if USE_SG
		(*sIter)->SendDeferredSG();
#else
		(*sIter)->SendDeferred();
#endif
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
