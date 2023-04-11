#pragma once
#include "StressTestClient.h"
#include "ClientSession.h"
#include "DelayManager.h"
#include <cmath>

StressTestClient::StressTestClient(shared_ptr<IocpClient> client, int clientNum)
	: m_client(client)
	, m_initCursor()
	, m_clientNum(clientNum)
	, m_coreCnt(thread::hardware_concurrency())
	, m_jobCnt(ceil(static_cast<double>(clientNum) / m_coreCnt))
	, m_threads()
	, m_bConnect(false)
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
		for (int i = 0; i < m_coreCnt; i++)
		{
			m_threads.push_back(thread([i, this]()
				{
					UpdateSessions(i);
				}));
		}
		
		for (thread& t : m_threads)
			t.join();
		
		if (m_bConnect == false)
		{
			m_bConnect = true;
			this_thread::sleep_for(1000ms);
		}

		m_threads.clear();

		UpdateOutput();
	}

	m_client->JoinWorkerThreads();
}

void StressTestClient::UpdateSessions(int idx)
{
	if (m_bConnect == false)
	{
		ConnectToServer(idx, PACKET_SEND_DURATION);
	}
	else
	{
		SendPacketToServer(idx, PACKET_SEND_DURATION);
	}
	//this_thread::sleep_for(1s);
}

void StressTestClient::SendPacketToServer(int idx, int duration)
{
	set<shared_ptr<IocpSession>>& sessions = m_client->GetSessions();
	set<shared_ptr<IocpSession>>::iterator iter;

	int startTime = 0;
	int processTime = 0;

	int deltaTime = duration / m_clientNum;
	this_thread::sleep_for(::milliseconds(deltaTime * idx));

	int i = 0;
	for (iter = sessions.begin(); iter != sessions.end(); iter++)
	{
		if (i % m_jobCnt != idx)
		{
			i++;
			continue;
		}
		startTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		
		shared_ptr<ClientSession> cliSession = static_pointer_cast<ClientSession>(*iter);
		if (cliSession->m_bLogin)
		{
			cliSession->SendMove();
		}
		/*
		else if (cliSession->m_bConnect && cliSession->m_bStartLogin == false)
		{
			cliSession->SendLogin();
		}
		*/
		processTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - startTime;
		this_thread::sleep_for(::milliseconds(deltaTime * m_jobCnt - processTime));
		
		i++;
	}
}

void StressTestClient::ConnectToServer(int idx, int duration)
{
	int startTime = 0;
	int processTime = 0;

	int deltaTime = duration / m_clientNum;
	this_thread::sleep_for(::milliseconds(deltaTime * idx));

	for (int i = 0; i < m_jobCnt; i++)
	{
		if (i * m_coreCnt + idx >= m_clientNum)
			break;

		startTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		
		if (m_client->ConnectNewSession() == false)
		{
			HandleError("ConnectToServer");
			return;
		}

		processTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - startTime;
		this_thread::sleep_for(::milliseconds(deltaTime * m_jobCnt - processTime));
	}
}

void StressTestClient::InitOutput()
{
	CONSOLE_SCREEN_BUFFER_INFO presentCur;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur);

	m_initCursor.X = presentCur.dwCursorPosition.X;
	m_initCursor.Y = presentCur.dwCursorPosition.Y;

	cout << "Current Client-Server Packet Send-Recv Delay \n = \n";
	//cout << "Current Login Delay \n = \n";
	cout << "Current Server Processing Delay \n = \n\n";
}

void StressTestClient::UpdateOutput()
{
	MoveCursor(3, 1);
	cout << gDelayMgr.GetAvgDelay() / 1000 << " milliseconds     ";
	MoveCursor(3, 3);
	//cout << gDelayMgr.GetAvgLoginDelay() / 1000 << " milliseconds     ";
	//MoveCursor(3, 5);
	cout << gDelayMgr.GetAvgProcessTime() / 1000 << " milliseconds     " << endl;
}

void StressTestClient::MoveCursor(int x, int y)
{
	COORD cursor;
	cursor.X = x + m_initCursor.X;
	cursor.Y = y + m_initCursor.Y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}