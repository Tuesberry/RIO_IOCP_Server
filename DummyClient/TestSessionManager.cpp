#include "TestSessionManager.h"
#include "ClientSession.h"
#include "DelayManager.h"

TestSessionManager gTestSessionMgr;

/* --------------------------------------------------------
*	Method:		TestSessionManager::TestSessionManager
*	Summary:	Constructor
-------------------------------------------------------- */
TestSessionManager::TestSessionManager()
	: m_rwLock()
	, m_sessions()
	, m_connectCnt(0)
	, m_size(0)
{
}

/* --------------------------------------------------------
*	Method:		TestSessionManager::SendLogin
*	Summary:	send login packet
*				from client session to server
*	Args:		int id
*					client session id
------------------------------------------------------- */
bool TestSessionManager::SendLogin(int id)
{
	ReadLockGuard lock(m_rwLock);

	if (m_sessions.count(id) == 0)
		return false;

	shared_ptr<ClientSession> session = m_sessions.find(id)->second.lock();
	if (session)
	{
		if (!session->m_bLogin && session->m_bConnect && session->m_bStartLogin == false)
		{
			session->SendLogin();
		}
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		TestSessionManager::SendMove
*	Summary:	send move packet
*				from client session to server
*	Args:		int id
*					client session id
------------------------------------------------------- */
bool TestSessionManager::SendMove(int id)
{
	ReadLockGuard lock(m_rwLock);

	if (m_sessions.count(id) == 0)
		return false;

	shared_ptr<ClientSession> session = m_sessions.find(id)->second.lock();
	if (session)
	{
		session->SendMove();
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		TestSessionManager::SendPacket
*	Summary:	send move or login packet
*				from client session to server
*	Args:		int id
*					client session id
------------------------------------------------------- */
bool TestSessionManager::SendPacket(int id)
{
	ReadLockGuard lock(m_rwLock);

	if (m_sessions.count(id) == 0)
	{
		cout << "SendPacket Error, Id = " << id << endl;
		return false;
	}

	// get session
	shared_ptr<ClientSession> session = m_sessions.find(id)->second.lock();
	if (session)
	{
		if (session->m_bLogin)
		{
			session->SendMove();
		}
		else if (session->m_bConnect && session->m_bStartLogin == false)
		{
			session->SendLogin();
		}
		gDelayMgr.m_sendCnt.fetch_add(1);
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		TestSessionManager::Disconnect
*	Summary:	send disconnect packet to server
*	Args:		int id
*					client session id
------------------------------------------------------- */
void TestSessionManager::Disconnect(int id)
{
	ReadLockGuard lock(m_rwLock);

	if (m_sessions.count(id) == 0)
		return;

	// get session
	shared_ptr<ClientSession> session = m_sessions.find(id)->second.lock();
	if (session)
	{
		if (session->IsConnected())
		{
			session->Disconnect();
			m_connectCnt--;
		}
	}
}

/* --------------------------------------------------------
*	Method:		TestSessionManager::AddSession
*	Summary:	add session to session manager
*	Args:		int id
*					client session id
*				weak_ptr<ClientSession> session
*					session to be added
------------------------------------------------------- */
void TestSessionManager::AddSession(int id, weak_ptr<ClientSession> session)
{
	WriteLockGuard lock(m_rwLock);
	m_sessions.insert({ id, session });
	m_size++;
	m_connectCnt++;
}

/* --------------------------------------------------------
*	Method:		TestSessionManager::DeleteSession
*	Summary:	delete session from session manager
*	Args:		int id
*					session id to be deleted
------------------------------------------------------- */
void TestSessionManager::DeleteSession(int id)
{
	WriteLockGuard lock(m_rwLock);
	m_sessions.erase(id);
	m_size--;
}
