#pragma once
#include "Common.h"

#include "RioCommon.h"
#include "RioEvent.h"

class RioSession;

/* --------------------------------------------------------
*	class:		RioCore
*	Summary:	Handle Completion Queue
-------------------------------------------------------- */
class RioCore : public enable_shared_from_this<RioCore>
{
public:
	RioCore();
	RioCore(const RioCore& other) = delete;
	RioCore(RioCore&& other) = delete;
	RioCore& operator=(const RioCore& other) = delete;
	RioCore& operator=(RioCore&& other) = delete;
	~RioCore() = default;

	// InitRioCore
	bool InitRioCore(HANDLE iocpHandle);
	bool InitRioCore();

	// Rio Completion Queue
#if SEPCQ
	RIO_CQ& GetSendCompletionQueue() { return m_sendCQ; }
	RIO_CQ& GetRecvCompletionQueue() { return m_recvCQ; }

	bool DispatchSend();
	bool DispatchRecv();
#else
	RIO_CQ& GetCompletionQueue() { return m_rioCompletionQueue; }

	bool Dispatch();
#endif

	// rionotify
#if RIOIOCP
	bool SetRioNotify();
#endif

	// RQ : Deferred Send
	void DeferredSend();
	
	// session
	void AddSession(shared_ptr<RioSession> session);
	void ReleaseSession(shared_ptr<RioSession> session);

private:
	// session
	mutex m_sessionLock;
	set<shared_ptr<RioSession>> m_sessions;
	int m_sessionCnt;
	bool m_bInit;

	// CompletionQueue
#if RIOIOCP
	RioCQEvent m_rioCQEvent;
#endif

#if SEPCQ
	// CompletionQueue
	RIO_CQ m_sendCQ;
	RIO_CQ m_recvCQ;
	RIORESULT m_sendResults[MAX_RIO_RESULT];
	RIORESULT m_recvResults[MAX_RIO_RESULT];
#else
	RIO_CQ m_rioCompletionQueue;
	RIORESULT m_results[MAX_RIO_RESULT];
#endif
};