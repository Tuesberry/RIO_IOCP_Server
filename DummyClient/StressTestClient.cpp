#include "StressTestClient.h"
#include "ClientSession.h"
#include "DelayManager.h"
#include <cmath>

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
*				from idx'th client session to server
*	Args:		int idx
*					client session index
------------------------------------------------------- */
bool TestSessionManager::SendLogin(int idx)
{
	if (m_size <= idx)
		return false;

	ReadLockGuard lock(m_rwLock);
	shared_ptr<ClientSession> session = m_sessions[idx].lock();
	if (session)
	{
		session->SendLogin();
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		TestSessionManager::SendMove
*	Summary:	send move packet
*				from idx'th client session to server
*	Args:		int idx
*					client session index
------------------------------------------------------- */
bool TestSessionManager::SendMove(int idx)
{
	if (m_size <= idx)
		return false;

	ReadLockGuard lock(m_rwLock);
	shared_ptr<ClientSession> session = m_sessions[idx].lock();
	if (session)
	{
		session->SendMove();
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		TestSessionManager::SendPacket
*	Summary:	send move or login packet
*				from idx'th client session to server
*	Args:		int idx
*					client session index
------------------------------------------------------- */
bool TestSessionManager::SendPacket(int idx)
{
	if (m_size <= idx)
		return false;

	ReadLockGuard lock(m_rwLock);

	// get session
	shared_ptr<ClientSession> session = m_sessions[idx].lock();
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
*	Summary:	disconnect session
*	Args:		int idx
*					client session index
------------------------------------------------------- */
void TestSessionManager::Disconnect(int idx)
{
	if (m_size <= idx)
		return;

	ReadLockGuard lock(m_rwLock);

	// get session
	shared_ptr<ClientSession> session = m_sessions[idx].lock();
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
*	Args:		weak_ptr<ClientSession> session
*					session to be added
------------------------------------------------------- */
void TestSessionManager::AddSession(weak_ptr<ClientSession> session)
{
	WriteLockGuard lock(m_rwLock);
	m_sessions.push_back(session);
	m_size++;
	m_connectCnt++;
}

/* --------------------------------------------------------
*	Method:		TestSessionManager::DeleteSession
*	Summary:	delete session from session manager
*	Args:		int idx
*					session to be deleted
------------------------------------------------------- */
void TestSessionManager::DeleteSession(int idx)
{
	WriteLockGuard lock(m_rwLock);
	m_sessions.erase(m_sessions.begin() + idx);
	m_size--;
}

/* --------------------------------------------------------
*	Method:		StressTestClient::StressTestClient
*	Summary:	Constructor
*	Args:		shared_ptr<IocpClient> client
*					iocp client
*				int clientNum
*					number of clients
*				int threadCnt
*					number of threads used for stress test
------------------------------------------------------- */
StressTestClient::StressTestClient(shared_ptr<IocpClient> client, int clientNum, int threadCnt)
	: m_client(client)
	, m_initCursor()
	, m_clientNum(clientNum)
	, m_runClient(false)
	, m_startTime(0)
	, m_threadCnt(threadCnt)
	, m_jobCnt(ceil(static_cast<double>(clientNum) / m_threadCnt))
{
	// resize send time
	m_sendTime.resize(clientNum);
}

/* --------------------------------------------------------
*	Method:		StressTestClient::~StressTestClient
*	Summary:	Destructor
------------------------------------------------------- */
StressTestClient::~StressTestClient()
{
	m_client->StopClient();
}

/* --------------------------------------------------------
*	Method:		StressTestClient::RunClient
*	Summary:	run stress test client
------------------------------------------------------- */
void StressTestClient::RunClient()
{
	if (m_client->StartClient() == false)
		return;

	// run client
	m_client->RunClient();

	// start check
	m_runClient = true;
	m_startTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();

	// init output
	//InitOutput();

	// connect sessions
	ConnectToServer();

	// reset sendTime
	ResetSendTime();
		
	// create threads
	// these threads send packets to server
	for (int i = 0; i < m_threadCnt; i++)
	{
		gThreadMgr.CreateThread([=]() 
			{
				while (true)
				{
					if (m_runClient)
					{
						SendToServer(i);
					}
				}
			});
	}
	
	while (true)
	{
		// update output
		//UpdateOutput();
		// check time
		if (m_runClient)
		{
			int workingTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count() - m_startTime;
			if (workingTime > STRESS_TEST_TIME_SEC)
			{
				m_runClient = false;
			}
		}
		else
		{
			TestStopOutput();
		}
		// sleep thread
		this_thread::sleep_for(100ms);
	}
}

/* --------------------------------------------------------
*	Method:		StressTestClient::ConnectToServer
*	Summary:	connect clients to server 
*				as the specified number of clients
------------------------------------------------------- */
void StressTestClient::ConnectToServer()
{
	for (int i = 0; i < m_clientNum; i++)
	{
		if (m_client->ConnectNewSession() == false)
		{
			HandleError("ConnectToServer");
			return;
		}
		this_thread::yield();
	}
}

/* --------------------------------------------------------
*	Method:		StressTestClient::DisconnectFromServer
*	Summary:	disconnect from server
*	Args:		int idx
*					execute thread index
------------------------------------------------------- */
void StressTestClient::DisconnectFromServer(int idx)
{
	int sIdx = 0;

	for (int i = 0; i < m_jobCnt; i++)
	{
		sIdx = i * m_threadCnt + idx;
		if (sIdx >= m_clientNum)
			break;

		if ((duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_sendTime[sIdx]) < PACKET_SEND_DURATION)
			continue;

		//cout << duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_sendTime[sIdx] << endl;
		m_sendTime[sIdx] = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		gTestSessionMgr.Disconnect(sIdx);
	}
}

/* --------------------------------------------------------
*	Method:		StressTestClient::SendToServer
*	Summary:	send packet to server from idx'th thread
*	Args:		int idx
*					execute thread index
------------------------------------------------------- */
bool StressTestClient::SendToServer(int idx)
{
	if (m_runClient == false)
		return false;

	int sIdx = 0;
	
	for (int i = 0; i < m_jobCnt; i++)
	{
		sIdx = i * m_threadCnt + idx;
		if (sIdx >= m_clientNum)
			break;

		if ((duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_sendTime[sIdx]) < PACKET_SEND_DURATION)
			continue;

		//cout << duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_sendTime[sIdx] << endl;
		m_sendTime[sIdx] = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		gTestSessionMgr.SendPacket(sIdx);
	}
	this_thread::yield();
	return true;
}

/* --------------------------------------------------------
*	Method:		StressTestClient::ResetSendTime
*	Summary:	reset send time for stress test
*				after this, session's send time distributes
*				with constant time delay
------------------------------------------------------- */
void StressTestClient::ResetSendTime()
{
	int startTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	double deltaTime = PACKET_SEND_DURATION / (double)m_clientNum;
	for (int i = 0; i < m_clientNum; i++)
	{
		m_sendTime[i] = startTime + deltaTime * i;
	}
}

/* --------------------------------------------------------
*	Method:		StressTestClient::InitOutput
*	Summary:	get init cursor and init console output
------------------------------------------------------- */
void StressTestClient::InitOutput()
{
	CONSOLE_SCREEN_BUFFER_INFO presentCur;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur);

	m_initCursor.X = presentCur.dwCursorPosition.X;
	m_initCursor.Y = presentCur.dwCursorPosition.Y;

	cout << "Current Client-Server Packet Send-Recv Delay \n = \n";
	cout << "Current Send Delay \n = \n";
	cout << "Current Recv Delay \n = \n";
	cout << "Current Login Delay \n = \n";
	cout << "Current Server Processing Delay \n = \n";
	cout << "Current Recv Packet Count \n = \n";
	cout << "Current Send Packet Count \n = \n";
	cout << "Current Connection Cnt \n = \n";
}


/* --------------------------------------------------------
*	Method:		StressTestClient::UpdateOutput
*	Summary:	update console output
------------------------------------------------------- */
void StressTestClient::UpdateOutput()
{
	MoveCursor(3, 1);
	cout << gDelayMgr.m_avgSendRecvDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 3);
	cout << gDelayMgr.m_avgSendingDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 5);
	cout << gDelayMgr.m_avgReceivingDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 7);
	cout << gDelayMgr.m_avgLoginDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 9);
	cout << gDelayMgr.m_avgProcessDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 11);
	cout << gDelayMgr.m_recvCnt.load() << "    ";
	MoveCursor(3, 13);
	cout << gDelayMgr.m_sendCnt.load() << "    \n";
}

/* --------------------------------------------------------
*	Method:		StressTestClient::TestStopOutput
*	Summary:	execute when stress test stop
------------------------------------------------------- */
void StressTestClient::TestStopOutput()
{
	MoveCursor(0, 11);
	cout << "STOP STRESS TEST!!" << endl;
}

/* --------------------------------------------------------
*	Method:		StressTestClient::MoveCursor
*	Summary:	move console cursor
*	Args:		int x
*					position x
*				int y
*					position y
------------------------------------------------------- */
void StressTestClient::MoveCursor(int x, int y)
{
	COORD cursor;
	cursor.X = x + m_initCursor.X;
	cursor.Y = y + m_initCursor.Y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}