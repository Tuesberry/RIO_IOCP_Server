#pragma once
#include "Common.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "SockAddress.h"

class IocpService;

/*--------------
	Session
---------------*/

class IocpSession : public IocpObject
{
	friend class IocpListener;
	friend class IocpCore;
	friend class IocpService;

public:
	IocpSession();
	virtual ~IocpSession();

public:
	/* �ܺο��� ��� */
	void Send(BYTE* buffer, int len);
	bool Connect();
	void Disconnect(const WCHAR* cause);

	shared_ptr<IocpService>	GetService() { return m_service.lock(); }
	void SetService(shared_ptr<IocpService> service) { m_service = service; }

public:
	/* ���� ���� */
	void SetAddress(SockAddress address) { m_Address = address; }
	SockAddress GetAddress() { return m_Address; }
	SOCKET GetSocket() { return m_socket; }
	bool IsConnected() { return m_connected; }
	shared_ptr<IocpSession> GetSessionRef() { return static_pointer_cast<IocpSession>(shared_from_this()); }

private:
	/* �������̽� ���� */
	virtual HANDLE GetHandle() override;
	virtual void ExecuteTask(class IocpEvent* iocpEvent, int bytesTrasferred = 0) override;

private:
	/* ���� ���� */
	bool RegisterConnect();
	bool RegisterDisconnect();
	void RegisterRecv();
	void RegisterSend(SendEvent* sendEvent);

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int numOfBytes);
	void ProcessSend(SendEvent* sendEvent, int numOfBytes);

	void HandleError(int errorCode);

protected:
	/* ������ �ڵ忡�� ������ */
	virtual void OnConnected() { }
	virtual int	OnRecv(BYTE* buffer, int len) { return len; }
	virtual void OnSend(int len) { }
	virtual void OnDisconnected() { }

public:
	// TEMP
	BYTE m_recvBuffer[1000];

	// Circular Buffer [             ]
	//char _sendBuffer[1000];
	//int32 _sendLen = 0;

private:
	weak_ptr<IocpService> m_service;
	SOCKET m_socket = INVALID_SOCKET;
	SockAddress	m_Address = {};
	atomic<bool> m_connected = false;

private:

	/* ���� ���� */

	/* �۽� ���� */

private:
	/* IocpEvent ���� */
	ConnectEvent m_connectEvent;
	DisconnectEvent	m_disconnectEvent;
	RecvEvent m_recvEvent;
};