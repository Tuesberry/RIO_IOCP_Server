#pragma once
#include "Common.h"

class RioCore;
class RioSession;

/* ----------------------------
*		RioServer
---------------------------- */
using SessionFactory = function<shared_ptr<RioSession>(void)>;

class RioServer
{
public:
	RioServer(SessionFactory sessionFactory);

	RioServer() = delete;
	RioServer(const RioServer& other) = delete;
	RioServer(RioServer&& other) = delete;
	RioServer& operator=(const RioServer& other) = delete;
	RioServer& operator=(RioServer&& other) = delete;
	~RioServer();

	bool InitServer();
	bool StartServer();

private:
	// listener
	bool InitListener();
	bool StartListener();
	bool Accept();

	// rioCore
	bool InitCore();
	bool StartCoreWork();

private:
	// session
	SessionFactory m_sessionFactory;

	// listener
	SOCKET m_listener;
	bool m_bInitListener;

	// thread
	vector<thread> m_threads;

	// RioCore
	vector<shared_ptr<RioCore>> m_rioCores;
	bool m_bInitCore;
	int m_coreCnt;
	int m_currAllocCoreNum;
};