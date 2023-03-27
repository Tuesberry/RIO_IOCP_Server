#pragma once
#include "Common.h"
#include "IocpCore.h"
#include "SockAddress.h"

class AcceptEvent;
class IocpServer;

class IocpListener : public IocpObject
{
public:
	IocpListener(shared_ptr<IocpServer> ownServer);

	IocpListener() = delete;
	IocpListener(const IocpListener& other) = delete;
	IocpListener(IocpListener&& other) = delete;
	IocpListener& operator=(const IocpListener& other) = delete;
	IocpListener& operator=(IocpListener&& other) = delete;
	~IocpListener();

	bool StartAccept();
	void CloseSocket();

	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

public:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(IocpEvent* iocpEvent, int bytesTransferred = 0) override;

private:
	SOCKET m_listener;
	vector<AcceptEvent*> m_acceptEvents;
	shared_ptr<IocpServer> m_ownerServer;
};