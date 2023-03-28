#pragma once
#include "Common.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "SockAddress.h"
#include "RecvBuffer.h"
#include "SendBuffer.h"

class IocpService;

/* ----------------------------
*		IocpSession
---------------------------- */

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
	shared_ptr<IocpService> GetService() { return m_iocpService; }

	bool IsConnected() { return m_bConnected; }

	// set
	void SetAddr(SOCKADDR_IN addr) { m_address = SockAddress(addr); }
	void SetAddr(SockAddress sockAddr) { m_address = sockAddr; }
	void SetService(shared_ptr<IocpService> iocpService) { m_iocpService = iocpService; }

	// networking
	bool Connect();
	void Disconnect();
	void Send(shared_ptr<SendBuffer> sendBuffer);

public:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(IocpEvent* iocpEvent, int bytesTransferred = 0) override;

public:
	bool RegisterConnect();
	void RegisterDisconnect();
	void RegisterRecv();
	void RegisterSend();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int bytesTransferred);
	void ProcessSend(int bytesTransferred);

protected:
	virtual void OnConnected(){}
	virtual int OnRecv(BYTE* buffer, int len) final;
	virtual void OnRecvPacket(BYTE* buffer, int len) abstract;
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
	SendEvent m_sendEvent;

	// iocp service
	shared_ptr<IocpService> m_iocpService;

public:
	// recvBuffer
	RecvBuffer m_recvBuffer;

	// sendBufferQueue
	queue<shared_ptr<SendBuffer>> m_sendBufferQueue;
	mutex m_sendBufLock;
	atomic<bool> m_bSendRegistered;
};