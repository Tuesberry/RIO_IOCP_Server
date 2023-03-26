#pragma once
#include "Common.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "SockAddress.h"

class IocpSession : public IocpObject
{
public:
	IocpSession();
	IocpSession(const IocpSession& other) = delete;
	IocpSession(IocpSession&& other) = delete;
	IocpSession& operator=(const IocpSession& other) = delete;
	IocpSession& operator=(IocpSession&& other) = delete;
	~IocpSession();

public:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(IocpEvent* iocpEvent, int bytesTransferred = 0) override;

private:
	void RegisterConnect();
	void RegisterDisconnect();
	void RegisterRecv(RecvEvent* recvEvent);
	void RegisterSend();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv();
	void ProcessSend();

protected:
	virtual void OnConnected(){}
	virtual int OnRecv(BYTE* buffer, int len) { return len; }
	virtual void OnSend(int len){}
	virtual void OnDisconnected(){}

private:
	SOCKET m_socket = INVALID_SOCKET;
	SockAddress m_address = {};
	atomic<bool> m_bConnected = false;
};