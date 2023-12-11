#include "StressTestClient.h"
#include "../Session/ClientSession.h"
#include "DelayManager.h"
#include <cmath>
#include "TestSessionManager.h"
#include "Thread/ThreadManager.h"
#include "Network/IocpClient.h"

/* --------------------------------------------------------
*	Method:		StressTestClient::StressTestClient
*	Summary:	Constructor
*	Args:		shared_ptr<IocpClient> client
*				    dummy client for test
*				int clientNum
*					number of clients
*				ETestMode testMode
*					selecting stress test mode
*				int threadCnt
*					number of threads used for stress test
------------------------------------------------------- */
StressTestClient::StressTestClient(shared_ptr<IocpClient> client, int clientNum, ETestMode testMode, int threadCnt)
	: m_client(client)
	, m_initCursor()
	, m_bRunClient(false)
	, m_bDisconnectComplete(false)
	, m_bStopTest(false)
	, m_testMode(testMode)
	, m_startTime(0)
	, m_clientNum(clientNum)
	, m_increaseRate(INCREASE_RATE_DEFAULT)
	, m_threadCnt(threadCnt)
	, m_jobCnt(ceil(static_cast<double>(clientNum) / m_threadCnt))
	, m_sendTime()
{
}

/* --------------------------------------------------------
*	Method:		StressTestClient::~StressTestClient
*	Summary:	Destructor
------------------------------------------------------- */
StressTestClient::~StressTestClient()
{

}

/* --------------------------------------------------------
*	Method:		StressTestClient::RunClient
*	Summary:	run stress test client
------------------------------------------------------- */
void StressTestClient::RunClient()
{
	if (!RunDummyClient())
	{
		return;
	}

	if (!ConnectToServer(m_clientNum))
	{
		return;
	}

	CreateSenderThreads();

	switch (m_testMode)
	{
	case ETestMode::NORMAL:
		StartNormalMode();
		break;
	case ETestMode::INCREASE:
		StartIncreaseMode();
		break;
	default:
		break;
	}
}

/* --------------------------------------------------------
*	Method:		StressTestClient::ConnectToServer
*	Summary:	connect clients to server 
*				as the specified number of clients
*	Args:		int clientNum
*					number of clients to connect
------------------------------------------------------- */
bool StressTestClient::ConnectToServer(int clientNum)
{
	for (int i = 0; i < clientNum; i++)
	{
		if (!m_client->ConnectNewSession())
		{
			HandleError("ConnectToServer");
			return false;
		}
	}
	return true;
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
		{
			break;
		}

		if ((duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_sendTime[sIdx]) < PACKET_SEND_DURATION)
		{
			continue;
		}

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
	if (!m_bRunClient)
	{
		return false;
	}

	int sIdx = 0;
	
	for (int i = 0; i < m_jobCnt; i++)
	{
		sIdx = i * m_threadCnt + idx;
		if (sIdx >= m_clientNum)
		{
			break;
		}

		if ((duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_sendTime[sIdx]) < PACKET_SEND_DURATION)
		{
			continue;
		}

		m_sendTime[sIdx] = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		gTestSessionMgr.SendPacket(sIdx+1);
	}
	this_thread::yield();
	return true;
}

/* --------------------------------------------------------
*	Method:		StressTestClient::CreateSenderThreads
*	Summary:	create sender threads
------------------------------------------------------- */
void StressTestClient::CreateSenderThreads()
{
	for (int i = 0; i < m_threadCnt; i++)
	{
		gThreadMgr->CreateThread([=]()
			{
				while (true)
				{
					if (m_bRunClient)
					{
						SendToServer(i);
					}
					if (m_bStopTest)
					{
						break;
					}
				}
			});
	}
}

/* --------------------------------------------------------
*	Method:		StressTestClient::RunDummyClient
*	Summary:	start and run dummyClient's iocpCore
------------------------------------------------------- */
bool StressTestClient::RunDummyClient()
{
	return m_client->Start([&]() {
		while (true)
		{
			m_client->GetIocpCore()->Dispatch();
		}
		});
}

/* --------------------------------------------------------
*	Method:		StressTestClient::StartNormalMode
*	Summary:	start normal mode stress test
------------------------------------------------------- */
void StressTestClient::StartNormalMode()
{
	// start check
	m_bRunClient = true;
	m_startTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();

	// reset sendTime
	ResetSendTime();

	// init output
	InitOutput();

	// stress test time check
	while (m_bRunClient)
	{
		// update output
		UpdateOutput();

		// check working time
		int workingTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count() - m_startTime;
		if (workingTime > STRESS_TEST_TIME_SEC)
		{
			m_bRunClient = false;
			m_bStopTest = true;
		}

		this_thread::sleep_for(100ms);
	}

	// disconnect sessions
	for (int i = 0; i < m_clientNum; i++)
	{
		DisconnectFromServer(i);
	}
}

/* --------------------------------------------------------
*	Method:		StressTestClient::StartIncreaseMode
*	Summary:	start increase mode stress test
------------------------------------------------------- */
void StressTestClient::StartIncreaseMode()
{
	while (!m_bStopTest)
	{
		// reset delay manager
		gDelayMgr.Reset();

		// set client count
		m_clientNum += INCREASE_RATE_DEFAULT;

		// set job count
		m_jobCnt = ceil(static_cast<double>(m_clientNum) / m_threadCnt);

		// connect new sessions
		if (!ConnectToServer(INCREASE_RATE_DEFAULT))
		{
			break;
		}

		// reset send time
		ResetSendTime();

		// start check
		m_bRunClient = true;
		m_startTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();

		// stress test time check
		while (m_bRunClient)
		{
			int workingTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count() - m_startTime;
			if (workingTime > STRESS_TEST_DELTA_TIME_SEC)
			{
				m_bRunClient = false;
			}

			this_thread::sleep_for(100ms);
		}

		// output
		PrintOutput();

		// test pause
		this_thread::sleep_for(1s);
	}
}

/* --------------------------------------------------------
*	Method:		StressTestClient::ResetSendTime
*	Summary:	reset send time for stress test
*				after this, session's send time distributes
*				with constant time delay
------------------------------------------------------- */
void StressTestClient::ResetSendTime()
{
	// send time vector initialize
	m_sendTime.resize(m_clientNum);

	// set send time
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
	cout << "Current Send Packet Count \n = \n";
	cout << "Current Recv Packet Count \n = \n";
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
	cout << gDelayMgr.m_sendCnt.load() << "       ";
	MoveCursor(3, 13);
	cout << gDelayMgr.m_recvCnt.load() << "       \n";
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
*	Method:		StressTestClient::PrintOutput
*	Summary:	print output
------------------------------------------------------- */
void StressTestClient::PrintOutput()
{
	cout << m_clientNum << " ";
	cout << gDelayMgr.m_avgSendRecvDelay.GetAvgDelay() / 1000 << " ";
	cout << gDelayMgr.m_avgSendingDelay.GetAvgDelay() / 1000 << " ";
	cout << gDelayMgr.m_avgReceivingDelay.GetAvgDelay() / 1000 << " ";
	cout << gDelayMgr.m_avgProcessDelay.GetAvgDelay() / 1000 << ' ';
	cout << gDelayMgr.m_recvCnt.load() << " ";
	cout << gDelayMgr.m_sendCnt.load() << '\n';
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