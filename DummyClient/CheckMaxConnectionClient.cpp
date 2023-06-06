#include "CheckMaxConnectionClient.h"

CheckMaxConnectionClient::CheckMaxConnectionClient(shared_ptr<IocpClient> client, int threadCnt)
	: m_client(client)
	, m_initCursor()
	, m_clientNum(0)
	, m_increaseRate(INCREASE_RATE_DEFAULT)
	, m_bRunClient(false)
	, m_bStopTest(false)
	, m_startTime(0)
	, m_threadCnt(threadCnt)
	, m_jobCnt(0)
	, m_sendTime()
{
}

CheckMaxConnectionClient::~CheckMaxConnectionClient()
{
	m_client->StopClient();
}

void CheckMaxConnectionClient::RunClient()
{
	if (m_client->StartClient() == false)
		return;

	// run client
	m_client->RunClient();

	// create thread
	CreateSenderThreads();

}

void CheckMaxConnectionClient::CreateSenderThreads()
{
	// create threads
	// these threads send packets to server
	for (int i = 0; i < m_threadCnt; i++)
	{
		gThreadMgr.CreateThread([=]()
			{
				while (true)
				{
					if (m_bRunClient)
					{
						SendToServer(i);
					}
					if (m_bStopTest)
					{
						break;
					}
				}
			});
	}
}

bool CheckMaxConnectionClient::ConnectToServer(int clientNum)
{
	for (int i = 0; i < clientNum; i++)
	{
		if (m_client->ConnectNewSession() == false)
		{
			HandleError("ConnectToServer");
			return false;
		}
	}
	return true;
}