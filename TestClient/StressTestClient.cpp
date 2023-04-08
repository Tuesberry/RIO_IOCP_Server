#pragma once
#include "StressTestClient.h"
#include "ClientSession.h"
#include "DelayManager.h"

StressTestClient::StressTestClient(shared_ptr<IocpClient> client, int clientNum)
	: m_client(client)
	, m_initCursor()
	, m_clientNum(clientNum)
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
		UpdateSessions();
		UpdateOutput();
	}

	m_client->JoinWorkerThreads();
}

void StressTestClient::UpdateSessions()
{
	if (m_client->GetConnectCnt() == 0)
	{
		ConnectToServer(PACKET_SEND_DURATION);
	}
	else
	{
		SendPacketToServer(PACKET_SEND_DURATION);
	}
}

void StressTestClient::SendPacketToServer(int duration)
{
	set<shared_ptr<IocpSession>>& sessions = m_client->GetSessions();
	set<shared_ptr<IocpSession>>::iterator iter;

	int startTime = 0;
	int processTime = 0;

	for (iter = sessions.begin(); iter != sessions.end(); iter++)
	{
		startTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		shared_ptr<ClientSession> cliSession = static_pointer_cast<ClientSession>(*iter);
		
		if (cliSession->m_bLogin)
		{
			cliSession->SendMove();
			processTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - startTime;
			this_thread::sleep_for(::milliseconds((duration / m_clientNum) - processTime));
		}
		else
		{
			std::cout << "login not complete!" << std::endl;
		}
	}
}

void StressTestClient::ConnectToServer(int duration)
{
	int startTime = 0;
	int processTime = 0;

	for (int i = 0; i < m_clientNum; i++)
	{
		startTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		if (m_client->ConnectNewSession() == false)
		{
			HandleError("ConnectToServer");
			return;
		}
		processTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - startTime;
		this_thread::sleep_for(::milliseconds((duration / m_clientNum) - processTime));
	}
}

void StressTestClient::InitOutput()
{
	CONSOLE_SCREEN_BUFFER_INFO presentCur;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur);

	m_initCursor.X = presentCur.dwCursorPosition.X;
	m_initCursor.Y = presentCur.dwCursorPosition.Y;

	cout << "Current Client-Server Packet Send-Recv Delay \n = " << endl;
}

void StressTestClient::UpdateOutput()
{
	MoveCursor(3, 1);
	cout << gDelayMgr.GetAvgDelay() / 1000 << " milliseconds   \n";
}

void StressTestClient::MoveCursor(int x, int y)
{
	COORD cursor;
	cursor.X = x + m_initCursor.X;
	cursor.Y = y + m_initCursor.Y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}

