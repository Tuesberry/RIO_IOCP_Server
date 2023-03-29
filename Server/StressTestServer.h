#pragma once
#include "CoreCommon.h"
#include "Network/IocpServer.h"

class StressTestServer
{
public:
	StressTestServer() = delete;
	StressTestServer(shared_ptr<IocpServer> server);

	StressTestServer(const StressTestServer& other) = delete;
	StressTestServer(StressTestServer&& other) = delete;
	StressTestServer& operator=(const StressTestServer& other) = delete;
	StressTestServer& operator=(StressTestServer&& other) = delete;
	~StressTestServer();

	void RunServer();

private:
	// draw
	void Draw();
	void InitDraw();

	// draw client session position
	void DrawMap();
	bool UpdateMap();
	//COORD PosToMapCursor();

	// set cursor
	void MoveCursor(int x, int y);

private:
	shared_ptr<IocpServer> m_server;

};