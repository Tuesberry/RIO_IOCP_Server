#include "StressTestClient.h"
#include "ClientSession.h"
#include "DelayManager.h"
#include <cmath>

TestSessionManager gTestSessionMgr;

TestSessionManager::TestSessionManager()
	: m_sessions()
	, m_size(0)
{
}

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

bool TestSessionManager::SendPacket(int idx)
{
	if (m_size <= idx)
		return false;

	ReadLockGuard lock(m_rwLock);
	shared_ptr<ClientSession> session = m_sessions[idx].lock();
	if (session)
	{
		if (session->m_bLogin)
		{
			session->SendMove();
		}
		else if (session->m_bStartLogin == false)
		{
			session->SendLogin();
		}
		gDelayMgr.m_sendCnt.fetch_add(1);
	}

	return true;
}

void TestSessionManager::AddSession(weak_ptr<ClientSession> session)
{
	WriteLockGuard lock(m_rwLock);
	m_sessions.push_back(session);
	m_size++;
}

void TestSessionManager::DeleteSession(int idx)
{
	WriteLockGuard lock(m_rwLock);
	m_sessions.erase(m_sessions.begin() + idx);
	m_size--;
}


StressTestClient::StressTestClient(shared_ptr<IocpClient> client, int clientNum)
	: m_client(client)
	, m_initCursor()
	, m_clientNum(clientNum)
	//, m_coreCnt(thread::hardware_concurrency())
	, m_coreCnt(2)
	, m_jobCnt(ceil(static_cast<double>(clientNum) / m_coreCnt))
{
	m_sendTime.resize(clientNum);
}

StressTestClient::~StressTestClient()
{
	m_client->StopClient();
}

void StressTestClient::RunServer()
{
	if (m_client->StartClient() == false)
		return;

	// run client
	m_client->RunClient();
	
	// init output
	InitOutput();

	// connect sessions
	ConnectToServer();

	// reset sendTime
	ResetSendTime();
		
	// create threads
	// these threads send packets to server
	for (int i = 0; i < m_coreCnt; i++)
	{
		gThreadMgr.CreateThread([=]() 
			{
				while (true)
				{
					SendToServer(i);
				}
			});
	}

	while (true)
	{
		UpdateOutput();
		this_thread::sleep_for(1s);
	}
}

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

void StressTestClient::SendToServer(int idx)
{
	int sIdx = 0;
	
	for (int i = 0; i < m_jobCnt; i++)
	{
		sIdx = i * m_coreCnt + idx;
		if (sIdx >= m_clientNum)
			break;

		if ((duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_sendTime[sIdx]) < PACKET_SEND_DURATION)
			continue;

		//cout << duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_sendTime[sIdx] << endl;
		m_sendTime[sIdx] = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		gTestSessionMgr.SendPacket(sIdx);
	}
	this_thread::yield();
}

void StressTestClient::ResetSendTime()
{
	int startTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	double deltaTime = PACKET_SEND_DURATION / (double)m_clientNum;
	for (int i = 0; i < m_clientNum; i++)
	{
		m_sendTime[i] = startTime + deltaTime * i;
	}
}

void StressTestClient::InitOutput()
{
	CONSOLE_SCREEN_BUFFER_INFO presentCur;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur);

	m_initCursor.X = presentCur.dwCursorPosition.X;
	m_initCursor.Y = presentCur.dwCursorPosition.Y;

	cout << "Current Client-Server Packet Send-Recv Delay \n = \n";
	cout << "Current Login Delay \n = \n";
	cout << "Current Server Processing Delay \n = \n";
	cout << "Current Recv Packet Count \n = \n";
	cout << "Current Send Packet Count \n = \n";
}

void StressTestClient::UpdateOutput()
{
	MoveCursor(3, 1);
	cout << gDelayMgr.m_avgSendingDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 3);
	cout << gDelayMgr.m_avgLoginDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 5);
	cout << gDelayMgr.m_avgProcessDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 7);
	cout << gDelayMgr.m_recvCnt.load() << "    ";
	MoveCursor(3, 9);
	cout << gDelayMgr.m_sendCnt.load() << "    \n";
}

void StressTestClient::MoveCursor(int x, int y)
{
	COORD cursor;
	cursor.X = x + m_initCursor.X;
	cursor.Y = y + m_initCursor.Y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}