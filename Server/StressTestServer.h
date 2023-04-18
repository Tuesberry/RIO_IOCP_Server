#pragma once

#include "Common.h"

#if IOCP
#include "Network/IocpServer.h"
#else RIO
#include "Network/RioServer.h"
#endif // IOCP

class StressTestServer
{
public:
#if IOCP
	StressTestServer(shared_ptr<IocpServer> server);
#else RIO
	StressTestServer(shared_ptr<RioServer> server);
#endif

public:
	StressTestServer() = delete;
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
#if IOCP
	shared_ptr<IocpServer> m_server;
#else RIO
	shared_ptr<RioServer> m_server;
#endif
};