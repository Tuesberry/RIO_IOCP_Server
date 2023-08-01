#pragma once

#include "Common.h"

#include "Network/IocpClient.h"
#include "Thread/RWLock.h"

class ClientSession;

/* --------------------------------------------------------
*	class:		TestSessionManager
*	Summary:	session manager for stress test
-------------------------------------------------------- */

class TestSessionManager
{
public:
	TestSessionManager();
	TestSessionManager(const TestSessionManager& other) = delete;
	TestSessionManager(TestSessionManager&& other) = delete;
	TestSessionManager& operator=(const TestSessionManager& other) = delete;
	TestSessionManager& operator=(TestSessionManager&& other) = delete;
	~TestSessionManager() = default;

	bool IsEmpty() { return m_connectCnt == 0; }

	bool SendLogin(int id);
	bool SendMove(int id);
	bool SendPacket(int id);

	void Disconnect(int id);

	void AddSession(int id, weak_ptr<ClientSession> session);
	void DeleteSession(int id);

	atomic<int> m_loginNum;

private:
	RWLock m_rwLock;
	map<int, weak_ptr<ClientSession>> m_sessions;

	int m_size;
	int m_connectCnt;
};

extern TestSessionManager gTestSessionMgr;