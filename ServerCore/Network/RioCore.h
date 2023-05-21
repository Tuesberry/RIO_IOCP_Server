#pragma once
#include "Common.h"

#include "RioEvent.h"

class RioSession;

/* --------------------------------------------------------
*	class:		RioCore
*	Summary:	Handle Completion Queue
-------------------------------------------------------- */
class RioCore : public enable_shared_from_this<RioCore>
{
public:
	static enum
	{
		MAX_CQ_SIZE = 163840,
		MAX_RIO_RESULT = 256,
	};

public:
	RioCore();
	RioCore(const RioCore& other) = delete;
	RioCore(RioCore&& other) = delete;
	RioCore& operator=(const RioCore& other) = delete;
	RioCore& operator=(RioCore&& other) = delete;
	~RioCore() = default;

	// get
	RIO_CQ& GetCompletionQueue() { return m_rioCompletionQueue; }

	// InitRioCore
	bool InitRioCore(HANDLE iocpHandle);

	// Rio Completion Queue
	bool Dispatch();
	bool SetRioNotify();

	// RQ : Deferred Send
	void DeferredSend();
	
	// session
	void AddSession(shared_ptr<RioSession> session);
	void ReleaseSession(shared_ptr<RioSession> session);

private:
	// CompletionQueue
	RioCQEvent m_rioCQEvent;
	RIO_CQ m_rioCompletionQueue;
	RIORESULT m_results[MAX_RIO_RESULT];

	// session
	mutex m_sessionLock;
	set<shared_ptr<RioSession>> m_sessions;
	int m_sessionCnt;

	bool m_bInit;
};