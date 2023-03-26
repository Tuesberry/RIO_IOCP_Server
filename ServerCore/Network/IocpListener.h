#pragma once
#include "Common.h"
#include "IocpCore.h"

class AcceptEvent;

class IocpListener : public IocpObject
{
public:
	IocpListener();
	IocpListener(const IocpListener& other) = delete;
	IocpListener(IocpListener&& other) = delete;
	IocpListener& operator=(const IocpListener& other) = delete;
	IocpListener& operator=(IocpListener&& other) = delete;
	~IocpListener();

	bool StartAccept(unsigned int portNum);
	void CloseSocket();

	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

public:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(IocpEvent* iocpEvent, int bytesTransferred = 0) override;

private:
	SOCKET m_listener;
};