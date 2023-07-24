#pragma once

#include "Common.h"
#include "RioCommon.h"

#include "SockAddress.h"

class RioCore;
class RioSession;

// rio session
using RIOSessionFactory = function<shared_ptr<RioSession>(void)>;

// jobQueue logic
using JobQueueLogicFunc = function<void(void)>;

/* --------------------------------------------------------
*	class:		RioServer
*	Summary:	registered i/o server
-------------------------------------------------------- */
class RioServer
{
public:
	RioServer(RIOSessionFactory sessionFactory, SockAddress address);

	RioServer() = delete;
	RioServer(const RioServer& other) = delete;
	RioServer(RioServer&& other) = delete;
	RioServer& operator=(const RioServer& other) = delete;
	RioServer& operator=(RioServer&& other) = delete;
	~RioServer();

	// server running
	bool InitServer();
	bool RunServer();
	void StopServer();

	// jobqueue logic
	void SetJobQueueLogic(JobQueueLogicFunc func) { m_jobQueueLogicFunc = func; }

	// get
	int GetConnectCnt() { return m_sessionCnt; }

private:
	// listener
	bool InitListener();
	bool StartListener();
	bool Accept();

	// rioCore
	bool InitCore();
	bool StartCoreWork();

	// iocp
	bool Dispatch();
	bool CreateIocpHandle();

private:
	// Session 
	RIOSessionFactory m_sessionFactory;
	atomic<int> m_sessionCnt;

	// Listener
	SOCKET m_listener;
	SockAddress m_address;
	bool m_bInitListener;

	// RIO CORE
	vector<shared_ptr<RioCore>> m_rioCores;
	bool m_bInitCore;
	int m_coreCnt;
	int m_currAllocCoreNum;

	// JobQueue Logic
	JobQueueLogicFunc m_jobQueueLogicFunc;

#if RIOIOCP
	HANDLE m_iocpHandle;
#endif // RIOIOCP
};