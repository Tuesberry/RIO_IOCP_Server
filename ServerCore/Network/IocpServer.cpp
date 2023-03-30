#include "IocpServer.h"
#include "IocpListener.h"
#include "IocpSession.h"

IocpServer::IocpServer(
	shared_ptr<IocpCore> iocpCore, 
	SessionFactory sessionFactory,
	SockAddress serverAddress, 
	int maxSessionCnt,
	int multipleThreadNum
)
	: IocpService(ServiceType::SERVER, iocpCore, sessionFactory, serverAddress, maxSessionCnt, multipleThreadNum)
	, m_iocpListener(nullptr)
{
}

IocpServer::~IocpServer()
{
	StopServer();
}

bool IocpServer::StartServer()
{
	if (CanStart() == false)
		return false;

	m_iocpListener = make_shared<IocpListener>(static_pointer_cast<IocpServer>(shared_from_this()));
	if (m_iocpListener == nullptr)
		return false;

	// start listener
	if (m_iocpListener->StartAccept() == false)
		return false;

	// set server start status
	m_bStart = true;

	return true;
}

void IocpServer::RunServer()
{
	if (CanStart() == false)
	{
		HandleError("RunServer");
		return;
	}

	// create threads
	CreateWorkerThreads();

	// TODO : server command check thread
}

void IocpServer::StopServer()
{
	// close listener socket
	m_iocpListener->CloseSocket();

	// DisconnectAllSessions
	DisconnectAllSession();

	// join threads
	JoinWorkerThreads();

	// server stop
	m_bStart = false;
}

void IocpServer::AddNewClient(int id)
{
	lock_guard<mutex> lock(m_clientInfoLock);
	CLIENT_INFO tempInfo = { 0,0 };
	m_clientInfo.insert(make_pair(id, tempInfo));
}

void IocpServer::RemoveClient(int id)
{
	lock_guard<mutex> lock(m_clientInfoLock);
	m_clientInfo.erase(id);
}

void IocpServer::SetClientPos(int id, int posX, int posY)
{
	//lock_guard<mutex> lock(m_clientInfoLock);
	auto iterCInfo = m_clientInfo.find(id);
	iterCInfo->second.posX = posX;
	iterCInfo->second.posY = posY;
}

bool IocpServer::GetClientInfo(int id, CLIENT_INFO& info)
{
	//lock_guard<mutex> lock(m_clientInfoLock);

	auto iterCInfo = m_clientInfo.find(id);

	if (iterCInfo == m_clientInfo.end())
		return false;

	info.posX = iterCInfo->second.posX;
	info.posY = iterCInfo->second.posY;

	return true;
}
