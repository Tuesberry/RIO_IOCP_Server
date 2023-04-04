#pragma once
#include "StressTestClient.h"

StressTestClient::StressTestClient(shared_ptr<IocpClient> client)
	: m_client(client)
	, m_connectStartTime()
	, m_lastConnectTime()
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
	bool expected = true;
	if (m_client->m_bCanConnected.compare_exchange_weak(expected, false) == true)
	{
		// check last connection time
		m_lastConnectTime = duration_cast<microseconds>(m_client->m_lastConnectTime - m_connectStartTime);

		if (m_lastConnectTime.count() > CONNECT_TIME_LIMIT_MS)
			return;

		//cout << sec.count() << endl;
		//cout << duration_cast<milliseconds>(m_client->m_lastConnectTime).count() << endl;

		// set new connection
		m_connectStartTime = high_resolution_clock::now();

		if (m_client->ConnectNewSession() == false)
			return;
	}
}

void StressTestClient::InitOutput()
{
	CONSOLE_SCREEN_BUFFER_INFO presentCur;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur);

	m_initCursor.X = presentCur.dwCursorPosition.X;
	m_initCursor.Y = presentCur.dwCursorPosition.Y;

	cout << "Connection Time Limit : " << CONNECT_TIME_LIMIT_MS << " Microseconds " << endl;
	cout << "Last Connection Time : " << endl;
}

void StressTestClient::UpdateOutput()
{
	MoveCursor(23, 1);
	cout << m_lastConnectTime.count() << " Microseconds   \n";
}

void StressTestClient::MoveCursor(int x, int y)
{
	COORD cursor;
	cursor.X = x + m_initCursor.X;
	cursor.Y = y + m_initCursor.Y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}
