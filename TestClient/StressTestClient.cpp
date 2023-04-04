#pragma once
#include "StressTestClient.h"

DelayManager gDelayMgr;

DelayManager::DelayManager()
	: m_connectionDelay(CONNECT_DELAY_INIT_MS)
	, m_avgDelay(0)
	, m_delayCnt(0)
	, m_bCanConnect(true)
{
}

void DelayManager::UpdateDelay(unsigned int delay)
{
	if (delay > DELAY_LIMIT_MS)
	{
		m_connectionDelay++;
	}
}

void DelayManager::UpdateAvgDelay(unsigned int delay, unsigned int prevDelay)
{
	lock_guard<mutex> lock(m_updateLock);
	m_avgDelay = (m_avgDelay * m_delayCnt - prevDelay + delay) / m_delayCnt;
}

void DelayManager::AddNewInAvgDelay(unsigned int delay)
{
	lock_guard<mutex> lock(m_updateLock);
	m_avgDelay = (m_avgDelay * m_delayCnt + delay) / (m_delayCnt + 1);
	m_delayCnt++;
}

void DelayManager::DeleteInAvgDelay(unsigned int delay)
{
	lock_guard<mutex> lock(m_updateLock);
	m_avgDelay = (m_avgDelay * m_delayCnt - delay) / (m_delayCnt - 1);
	m_delayCnt--;
}

void DelayManager::CheckDelay()
{
}

StressTestClient::StressTestClient(shared_ptr<IocpClient> client)
	: m_client(client)
	, m_initCursor()
{
}

StressTestClient::~StressTestClient()
{
	m_client->StopClient();
}

void StressTestClient::RunServer()
{
	if (m_client->StartClient() == false)
		return;

	m_client->RunClient();
	
	InitOutput();
	while (true)
	{
		UpdateConnection();
		UpdateOutput();
	}

	m_client->JoinWorkerThreads();
}

void StressTestClient::UpdateConnection()
{
	// connect 
	if (m_client->ConnectNewSession() == false)
		return;

	// connect delay
	this_thread::sleep_for(::milliseconds(gDelayMgr.m_connectionDelay));
	/*
	bool expected = true;
	if (m_client->m_bCanConnected.compare_exchange_weak(expected, false) == true)
	{
		// check last connection time
		m_lastConnectTime = duration_cast<microseconds>(m_client->m_lastConnectTime - m_connectStartTime);

		if (m_lastConnectTime.count() > CONNECT_TIME_LIMIT_MS)
		{
			m_client->DisconnectSession();
			return;
		}
			

		//cout << sec.count() << endl;
		//cout << duration_cast<milliseconds>(m_client->m_lastConnectTime).count() << endl;

		// set new connection
		m_connectStartTime = high_resolution_clock::now();

		if (m_client->ConnectNewSession() == false)
			return;
	}
	*/
}

void StressTestClient::InitOutput()
{
	CONSOLE_SCREEN_BUFFER_INFO presentCur;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur);

	m_initCursor.X = presentCur.dwCursorPosition.X;
	m_initCursor.Y = presentCur.dwCursorPosition.Y;

	cout << "Current Connection Delay : " << endl;
	cout << "Current Avg Delay : " << endl;
}

void StressTestClient::UpdateOutput()
{
	MoveCursor(25, 0);
	cout << gDelayMgr.m_connectionDelay << " milliseconds   \n";

	MoveCursor(23, 1);
	cout << gDelayMgr.m_avgDelay << " milliseconds   \n";

	cout << gDelayMgr.updateCnt << endl;
	cout << gDelayMgr.m_delayCnt << endl;

}

void StressTestClient::MoveCursor(int x, int y)
{
	COORD cursor;
	cursor.X = x + m_initCursor.X;
	cursor.Y = y + m_initCursor.Y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}

