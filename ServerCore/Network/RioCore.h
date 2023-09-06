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

	RIO_CQ& GetCompletionQueue() { return m_rioCompletionQueue; }

	// rio completion queue logic
	bool Dispatch();
	bool InitRioCore();
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
	RIO_CQ m_rioCompletionQueue;
	RIORESULT m_results[MAX_RIO_RESULT];
};