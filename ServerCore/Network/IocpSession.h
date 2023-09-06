#pragma once

#include "Common.h"
#include "IocpCommon.h"

#include "IocpCore.h"
#include "IocpEvent.h"
#include "SockAddress.h"
#include "NetBuffer.h"

class IocpService;

/* --------------------------------------------------------
*	class:		IocpSession
*	Summary:	Session managed by iocp core
-------------------------------------------------------- */
class IocpSession : public IocpObject
{
public:
	IocpSession();
	IocpSession(const IocpSession& other) = delete;
	IocpSession(IocpSession&& other) = delete;
	IocpSession& operator=(const IocpSession& other) = delete;
	IocpSession& operator=(IocpSession&& other) = delete;
	virtual ~IocpSession();

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
	void Send(shared_ptr<NetBuffer> sendBuffer);

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
	// override at the content level
	virtual void OnConnected(){}
	virtual int OnRecv(char* buffer, int len) final;
	virtual void OnRecvPacket(char* buffer, int len){}
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
	// RecvBuffer m_recvBuffer;
	NetBuffer m_recvBuffer;

	// sendBufferQueue
	queue<shared_ptr<NetBuffer>> m_sendBufferQueue;
	mutex m_sendBufLock;
	atomic<bool> m_bSendRegistered;
};