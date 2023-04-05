#pragma once
#include "StressTestClient.h"

DelayManager gDelayMgr;

DelayManager::DelayManager()
	: m_connectionDelay(CONNECT_DELAY_INIT_MICROS)
	, m_avgDelay(0)
	, m_delayCnt(0)
	, m_bCanConnect(true)
	, m_bDisconnect(false)
	, m_updateAvgLock()
	, m_updateDelayLock()
	, m_updateCnt(0)
{
}

void DelayManager::UpdateDelay(int delay)
{
	lock_guard<mutex> lock(m_updateAvgLock);
	if (delay > DELAY_LIMIT_MS)
	{
		//cout << delay << endl;
		m_connectionDelay+= 10;
	}
}

void DelayManager::UpdateAvgDelay(int delay, int prevDelay)
{
	lock_guard<mutex> lock(m_updateDelayLock);
	m_avgDelay = (long double)(m_avgDelay * m_delayCnt - prevDelay + delay) / m_delayCnt;
}

void DelayManager::AddNewInAvgDelay(int delay)
{
	lock_guard<mutex> lock(m_updateDelayLock);
	m_avgDelay = (long double)(m_avgDelay * m_delayCnt + delay) / ((long double)m_delayCnt + 1);
	m_delayCnt++;
}

void DelayManager::DeleteInAvgDelay(int delay)
{
	lock_guard<mutex> lock(m_updateDelayLock);
	m_avgDelay = (long double)(m_avgDelay * m_delayCnt - delay) / ((long double)m_delayCnt - 1);
	m_delayCnt--;
}

void DelayManager::UpdateConnectionStatus()
{
	// 평균 반응 시간 > 150 ms -> 접속 시도 중지
	if (m_bCanConnect)
	{
		if (m_avgDelay > AVG_DELAY_LIMIT_MS)
		{
			m_bCanConnect = false;
			m_bDisconnect = true;
		}
	}
	else
	{
		if (m_avgDelay < AVG_DELAY_LIMIT_MS)
		{
			m_bDisconnect = false;
		}
	}
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
	// check delay
	gDelayMgr.UpdateConnectionStatus();

	if (gDelayMgr.m_bCanConnect)
	{
		// connect 
		if (m_client->ConnectNewSession() == false)
			return;

		// connect delay
		this_thread::sleep_for(::microseconds(gDelayMgr.m_connectionDelay));
	}
	else if(gDelayMgr.m_bDisconnect)
	{
		// disconnect
		if (m_client->DisconnectSession() == false)
			return;

		// disconnect delay
		this_thread::sleep_for(::milliseconds(100));
	}

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
	cout << (long double)gDelayMgr.m_connectionDelay / 1000 << " milliseconds   \n";

	MoveCursor(23, 1);
	cout << gDelayMgr.m_avgDelay << " milliseconds   \n";
}

void StressTestClient::MoveCursor(int x, int y)
{
	COORD cursor;
	cursor.X = x + m_initCursor.X;
	cursor.Y = y + m_initCursor.Y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}

