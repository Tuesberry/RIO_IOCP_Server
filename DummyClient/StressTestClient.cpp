#include "StressTestClient.h"
#include "ClientSession.h"
#include "DelayManager.h"
#include <cmath>
#include "TestSessionManager.h"

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
	, m_bRunClient(false)
	, m_bDisconnectComplete(false)
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
	m_bRunClient = true;
	m_startTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();

	// init output
	InitOutput();

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
					if (m_bRunClient)
					{
						SendToServer(i);
					}
					if (m_bDisconnectComplete)
					{
						break;
					}
				}
			});
	}
	
	while (true)
	{
		// update output
		UpdateOutput();
		// check time
		if (m_bRunClient)
		{
			int workingTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count() - m_startTime;
			if (workingTime > STRESS_TEST_TIME_SEC)
			{
				m_bRunClient = false;
			}
		}
		else
		{
			TestStopOutput();
			break;
		}
		// sleep thread
		this_thread::sleep_for(100ms);
	}

	// disconnect all session
	for (int i = 0; i < m_clientNum; i++)
	{
		DisconnectFromServer(i);
	}

	// stop sending threads
	m_bDisconnectComplete = true;
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

		m_sendTime[sIdx] = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		gTestSessionMgr.Disconnect(sIdx+1);
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
	if (m_bRunClient == false)
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
		gTestSessionMgr.SendPacket(sIdx+1);
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
	__int64 startTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
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
	cout << "Current Server Processing | Move Player Delay \n = \n";
	cout << "Current Server Processing | Synchronize Player Count \n = \n";
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
	cout << gDelayMgr.m_avgUpdatePosDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 13);
	cout << gDelayMgr.m_avgSynchronizeMoveDelay.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 15);
	cout << gDelayMgr.m_recvCnt.load() << "       ";
	MoveCursor(3, 17);
	cout << gDelayMgr.m_sendCnt.load() << "       \n";
}

/* --------------------------------------------------------
*	Method:		StressTestClient::TestStopOutput
*	Summary:	execute when stress test stop
------------------------------------------------------- */
void StressTestClient::TestStopOutput()
{
	MoveCursor(0, 19);
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