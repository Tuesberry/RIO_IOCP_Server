#pragma once
#include "pch.h"
#include "StressTestServer.h"

StressTestServer::StressTestServer(shared_ptr<IocpServer> server)
	:m_server(server)
{
}

StressTestServer::~StressTestServer()
{
	m_server->StopServer();
}

void StressTestServer::RunServer()
{
	if (m_server->StartServer() == false)
	{
		return;
	}

	m_server->RunServer();

	InitDraw();
	while (true)
	{
		Draw();
	}

	m_server->JoinWorkerThreads();
}

void StressTestServer::Draw()
{
	int currCnt = m_server->GetConnectCnt();

	MoveCursor(25, 0);
	cout << currCnt << endl;
		
	//system("cls");
	MoveCursor(0, 3);
	CLIENT_INFO cInfo;
	bool result;
	for (int i = 1; i <= currCnt; i++)
	{
		result = m_server->GetClientInfo(i, cInfo);
		if (result == true)
		{
			cout << "Session " << i << " | posX : " << cInfo.posX << ", posY : " << cInfo.posY << endl;
		}
		else
		{
			m_server->RemoveClient(i);
		}
	}
}

void StressTestServer::InitDraw()
{
	cout << "Connected Client Count:\n" << endl;

	DrawMap();
}

void StressTestServer::DrawMap()
{
	for (int i = 0; i < 60; i++)
	{
		if (i == 0 || i == 59)
			cout << "+";
		else
			cout << "-";
	}
}

bool StressTestServer::UpdateMap()
{

	return false;
}

void StressTestServer::MoveCursor(int x, int y)
{
	COORD cursor;
	cursor.X = x;
	cursor.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}
