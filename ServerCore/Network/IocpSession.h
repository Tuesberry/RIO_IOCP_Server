#pragma once
#include "Common.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "SockAddress.h"

class IocpService;

class IocpSession : public IocpObject
{
public:
	IocpSession();
	IocpSession(const IocpSession& other) = delete;
	IocpSession(IocpSession&& other) = delete;
	IocpSession& operator=(const IocpSession& other) = delete;
	IocpSession& operator=(IocpSession&& other) = delete;
	~IocpSession();

	// get
	SOCKET GetSocket() { return m_socket; }
	SockAddress GetAddr() { return m_address; }

	// set
	void SetAddr(SOCKADDR_IN addr) { m_address = SockAddress(addr); }
	void SetAddr(SockAddress sockAddr) { m_address = sockAddr; }
	void SetService(shared_ptr<IocpService> iocpService) { m_iocpService = iocpService; }

	bool IsConnected() { return m_bConnected; }

	bool Connect();
	void Disconnect();
	void Send(BYTE* buffer, int len);

public:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(IocpEvent* iocpEvent, int bytesTransferred = 0) override;

public:
	bool RegisterConnect();
	void RegisterDisconnect();
	void RegisterRecv();
	void RegisterSend(BYTE* sendData, int dataLen);

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int bytesTransferred);
	void ProcessSend(SendEvent* sendEvent, int bytesTransferred);

protected:
	virtual void OnConnected(){}
	virtual int OnRecv(BYTE* buffer, int len) { return len; }
	virtual void OnSend(int len){}
	virtual void OnDisconnected(){}

private:
	// socket
	SOCKET m_socket;
	SockAddress m_address;
	atomic<bool> m_bConnected;

	// iocp event
	RecvEvent m_recvEvent;
	ConnectEvent m_connectEvent;
	DisconnectEvent m_disconnectEvent;

	// iocp service
	shared_ptr<IocpService> m_iocpService;

public:
	// recvBuffer
	BYTE m_recvBuffer[1000];
};