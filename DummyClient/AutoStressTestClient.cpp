#include "AutoStressTestClient.h"
#include "ClientSession.h"
#include "DelayManager.h"
#include <cmath>
#include "TestSessionManager.h"

/* --------------------------------------------------------
*	Method:		AutoStressTestClient::AutoStressTestClient
*	Summary:	Constructor
*	Args:		shared_ptr<IocpClient> client
*					iocp client
*				int clientNum
*					number of clients
*				int threadCnt
*					number of threads used for stress test
------------------------------------------------------- */
AutoStressTestClient::AutoStressTestClient(shared_ptr<IocpClient> client, int threadCnt)
	: m_client(client)
	, m_initCursor()
	, m_clientNum(0)
	, m_increaseRate(INCREASE_RATE_DEFAULT)
	, m_bRunClient(false)
	, m_bStopTest(false)
	, m_startTime(0)
	, m_threadCnt(threadCnt)
	, m_jobCnt(0)
	, m_sendTime()
{
}

/* --------------------------------------------------------
*	Method:		AutoStressTestClient::~AutoStressTestClient
*	Summary:	Destructor
------------------------------------------------------- */
AutoStressTestClient::~AutoStressTestClient()
{
	m_client->StopClient();
}

/* --------------------------------------------------------
*	Method:		CheckMaxPointClient::RunClient
*	Summary:	run stress test client
------------------------------------------------------- */
void AutoStressTestClient::RunClient()
{
	if (m_client->StartClient() == false)
		return;

	// run client
	m_client->RunClient();

	// START connection
	m_clientNum = START_NUM;
	if (!ConnectToServer(m_clientNum))
	{
		HandleError("ConnectToServer");
		return;
	}

	// create thread
	CreateSenderThreads();

	while (!m_bStopTest)
	{
		// reset delay manager
		gDelayMgr.Reset();

		// set client count
		m_clientNum += m_increaseRate;

		// set job count
		m_jobCnt = ceil(static_cast<double>(m_clientNum) / m_threadCnt);

		// set login only
		m_bSendLoginOnly = true;

		// connect new sessions
		if (!ConnectToServer(m_increaseRate))
		{
			HandleError("ConnectToServer");
			break;
		}

		// reset send time
		ResetSendTime();

		// start check
		m_bRunClient = true;
		m_startTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();

		// wait 1s -> send move packet start
		this_thread::sleep_for(1s);
		m_bSendLoginOnly = false;

		// stress test time check
		while (m_bRunClient)
		{
			int workingTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count() - m_startTime;
			if (workingTime > STRESS_TEST_TIME_SEC)
			{
				m_bRunClient = false;
			}

			this_thread::sleep_for(100ms);
		}

		// output
		PrintOutput();

		// test pause
		this_thread::sleep_for(5s);
	}


}

void AutoStressTestClient::CreateSenderThreads()
{
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
					if (m_bStopTest)
					{
						break;
					}
				}
			});
	}
}

bool AutoStressTestClient::ConnectToServer(int clientNum)
{
	for (int i = 0; i < clientNum; i++)
	{
		if (m_client->ConnectNewSession() == false)
		{
			HandleError("ConnectToServer");
			return false;
		}
	}
	return true;
}

void AutoStressTestClient::ResetSendTime()
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

bool AutoStressTestClient::SendToServer(int idx)
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

		m_sendTime[sIdx] = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();

		if (m_bSendLoginOnly)
		{
			gTestSessionMgr.SendLogin(sIdx + 1);
		}
		else
		{
			gTestSessionMgr.SendPacket(sIdx + 1);
		}
	}
	this_thread::yield();
	return true;
}

void AutoStressTestClient::PrintOutput()
{
	/*
	cout << "client num: " << m_clientNum << endl;
	cout << "Delay: " << gDelayMgr.m_avgSendRecvDelay.GetAvgDelay() / 1000 << " ms\n";
	cout << "Send Delay: " << gDelayMgr.m_avgSendingDelay.GetAvgDelay() / 1000 << " ms\n";
	cout << "Recv Delay: " << gDelayMgr.m_avgReceivingDelay.GetAvgDelay() / 1000 << " ms\n";
	cout << "Processing Delay: " << gDelayMgr.m_avgProcessDelay.GetAvgDelay() / 1000 << " ms\n";
	cout << "Send Count: " << gDelayMgr.m_sendCnt.load() << '\n';
	cout << "Recv Count: " << gDelayMgr.m_recvCnt.load() << '\n';
	cout << "=====================================================\n";
	*/

	cout << m_clientNum << " ";
	cout << gDelayMgr.m_avgSendRecvDelay.GetAvgDelay() / 1000 << " ";
	cout << gDelayMgr.m_avgSendingDelay.GetAvgDelay() / 1000 << " ";
	cout << gDelayMgr.m_avgReceivingDelay.GetAvgDelay() / 1000 << " ";
	cout << gDelayMgr.m_avgProcessDelay.GetAvgDelay() / 1000 << '\n';
}
