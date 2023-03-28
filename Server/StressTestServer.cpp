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

	while (true)
	{
		Draw();
	}

	m_server->JoinWorkerThreads();
}

void StressTestServer::Draw()
{
}
