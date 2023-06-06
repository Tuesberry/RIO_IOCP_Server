#include "StressTestServer.h"

#include "Room.h"
//#include "RoomOrigin.h"

#if IOCP
StressTestServer::StressTestServer(shared_ptr<IocpServer> server)
	:m_server(server)
{
}
#else RIO
StressTestServer::StressTestServer(shared_ptr<RioServer> server)
	: m_server(server)
{
}
#endif

StressTestServer::~StressTestServer()
{
	m_server->StopServer();
}

void StressTestServer::RunServer()
{
	if (m_server->InitServer() == false)
	{
		return;
	}

	m_server->RunServer();

	//InitDraw();
	while (true)
	{
		//Draw();
	}
}

void StressTestServer::Draw()
{
	int currCnt = m_server->GetConnectCnt();

	MoveCursor(25, 0);
	cout << currCnt << endl;

	MoveCursor(25, 1);
	cout << gRoom.GetLoginCnt() << endl;
}

void StressTestServer::InitDraw()
{
	cout << "Connected Client Count:" << endl;
	cout << "Login Client Count:" << endl;
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
