#include "CheckMaxPointClient.h"
#include "ClientSession.h"
#include "DelayManager.h"
#include <cmath>
#include "TestSessionManager.h"

/* --------------------------------------------------------
*	Method:		CheckMaxPointClient::CheckMaxPointClient
*	Summary:	Constructor
*	Args:		shared_ptr<IocpClient> client
*					iocp client
*				int clientNum
*					number of clients
*				int threadCnt
*					number of threads used for stress test
------------------------------------------------------- */
CheckMaxPointClient::CheckMaxPointClient(shared_ptr<IocpClient> client, int threadCnt)
	: m_client(client)
	, m_initCursor()
	, m_clientNum(0)
	, m_runClient(false)
	, m_startTime(0)
	, m_threadCnt(threadCnt)
	, m_jobCnt(ceil(static_cast<double>(0) / m_threadCnt))
{
	// resize send time
	m_sendTime.resize(0);
}

/* --------------------------------------------------------
*	Method:		CheckMaxPointClient::~CheckMaxPointClient
*	Summary:	Destructor
------------------------------------------------------- */
CheckMaxPointClient::~CheckMaxPointClient()
{
	m_client->StopClient();
}

/* --------------------------------------------------------
*	Method:		CheckMaxPointClient::RunClient
*	Summary:	run stress test client
------------------------------------------------------- */
void CheckMaxPointClient::RunClient()
{
	if (m_client->StartClient() == false)
		return;

	// run client
	m_client->RunClient();

	// start check
	m_runClient = true;
	m_startTime = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();

}
