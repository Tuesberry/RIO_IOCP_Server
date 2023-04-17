#pragma once

#include "Common.h"

class RioCore;
class RioSession;

// rio session
using RIOSessionFactory = function<shared_ptr<RioSession>(void)>;

/* --------------------------------------------------------
*	class:		RioServer
*	Summary:	registered i/o server
-------------------------------------------------------- */
class RioServer
{
public:
	RioServer(RIOSessionFactory sessionFactory);

	RioServer() = delete;
	RioServer(const RioServer& other) = delete;
	RioServer(RioServer&& other) = delete;
	RioServer& operator=(const RioServer& other) = delete;
	RioServer& operator=(RioServer&& other) = delete;
	~RioServer();

	// server running
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
	RIOSessionFactory m_sessionFactory;

	// listener
	SOCKET m_listener;
	bool m_bInitListener;

	// RioCore
	vector<shared_ptr<RioCore>> m_rioCores;
	bool m_bInitCore;
	int m_coreCnt;
	int m_currAllocCoreNum;
};