#pragma once
#include "Common.h"
#include "IocpCore.h"

class AcceptEvent;
class IocpServerService;

/* ----------------------------
*		IocpListener
---------------------------- */

class IocpListener : public IocpObject
{
public:
	IocpListener();
	IocpListener(const IocpListener& other) = delete;
	IocpListener(IocpListener&& other) = delete;
	IocpListener& operator=(const IocpListener& other) = delete;
	IocpListener& operator=(IocpListener&& other) = delete;
	~IocpListener();

	bool StartAccept(shared_ptr<IocpServerService> service);
	void CloseSocket();

public:
	virtual HANDLE GetHandle() override;
	virtual void ExecuteTask(class IocpEvent* iocpEvent, int bytesTransferred = 0) override;

private:
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

private:
	SOCKET m_socket;
	vector<AcceptEvent*> m_acceptEvents;
	shared_ptr<IocpServerService> m_service;
};