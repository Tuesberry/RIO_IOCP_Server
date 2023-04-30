#pragma once
#include "Common.h"

class RioSession;

/* --------------------------------------------------------
*	class:		RioCore
*	Summary:	Handle Completion Queue
-------------------------------------------------------- */
class RioCore
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

	// core work
	bool InitRioCore();
	bool Dispatch();
	
	// session
	void AddSession(shared_ptr<RioSession> session);
	void ReleaseSession(shared_ptr<RioSession> session);

private:
	// CompletionQueue
	RIO_CQ m_rioCompletionQueue;
	RIORESULT m_results[MAX_RIO_RESULT];

	// session
	mutex m_sessionLock;
	set<shared_ptr<RioSession>> m_sessions;
	int m_sessionCnt;

	// general
	bool m_bInit;
};