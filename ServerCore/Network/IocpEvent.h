#pragma once
#include "Common.h"

class IocpObject;
class IocpSession;

/* ----------------------------
*		IO_TYPE
---------------------------- */

enum class IO_TYPE
{
	ACCEPT,
	CONNECT,
	DISCONNECT,
	RECV,
	SEND
};

/* ----------------------------
*		IocpEvent
---------------------------- */

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent() = delete;
	IocpEvent(IO_TYPE type);
	IocpEvent(const IocpEvent& other) = delete;
	IocpEvent(IocpEvent&& other) = delete;
	IocpEvent& operator=(const IocpEvent& other) = delete;
	IocpEvent& operator=(IocpEvent&& other) = delete;
	~IocpEvent() = default;

	void Init();

public:
	IO_TYPE m_eventType;
	shared_ptr<IocpObject> m_owner;
};

/* ----------------------------
*		AcceptEvent
---------------------------- */

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() :IocpEvent(IO_TYPE::ACCEPT) {}

	AcceptEvent(const AcceptEvent& other) = delete;
	AcceptEvent(AcceptEvent&& other) = delete;
	AcceptEvent& operator=(const AcceptEvent& other) = delete;
	AcceptEvent& operator=(AcceptEvent&& other) = delete;
	~AcceptEvent() = default;

public:
	// accept socket session
	shared_ptr<IocpSession> m_session;
};

/* ----------------------------
*		ConnectEvent
---------------------------- */

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() :IocpEvent(IO_TYPE::CONNECT) {}

	ConnectEvent(const ConnectEvent& other) = delete;
	ConnectEvent(ConnectEvent&& other) = delete;
	ConnectEvent& operator=(const ConnectEvent& other) = delete;
	ConnectEvent& operator=(ConnectEvent&& other) = delete;
	~ConnectEvent() = default;
};

/* ----------------------------
*		DisconnectEvent
---------------------------- */

class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() :IocpEvent(IO_TYPE::DISCONNECT) {}

	DisconnectEvent(const DisconnectEvent& other) = delete;
	DisconnectEvent(DisconnectEvent&& other) = delete;
	DisconnectEvent& operator=(const DisconnectEvent& other) = delete;
	DisconnectEvent& operator=(DisconnectEvent&& other) = delete;
	~DisconnectEvent() = default;
};

/* ----------------------------
*		RecvEvent
---------------------------- */

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() :IocpEvent(IO_TYPE::RECV) {}

	RecvEvent(const RecvEvent& other) = delete;
	RecvEvent(RecvEvent&& other) = delete;
	RecvEvent& operator=(const RecvEvent& other) = delete;
	RecvEvent& operator=(RecvEvent&& other) = delete;
	~RecvEvent() = default;
};

/* ----------------------------
*		SendEvent
---------------------------- */

class SendEvent : public IocpEvent
{
public:
	SendEvent() :IocpEvent(IO_TYPE::SEND) {}

	SendEvent(const SendEvent& other) = delete;
	SendEvent(SendEvent&& other) = delete;
	SendEvent& operator=(const SendEvent& other) = delete;
	SendEvent& operator=(SendEvent&& other) = delete;
	~SendEvent() = default;
public:
	BYTE m_sendBuffer[1000];
};