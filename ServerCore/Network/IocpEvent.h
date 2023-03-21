#pragma once
#include "Common.h"

class IocpObject;
class IocpSession;

/* ----------------------------
*		EventType
---------------------------- */

enum class EventType : unsigned _int8
{
	CONNECT,
	DISCONNECT,
	ACCEPT,
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
	IocpEvent(EventType type);
	IocpEvent(const IocpEvent& other) = delete;
	IocpEvent(IocpEvent&& other) = delete;
	IocpEvent& operator=(const IocpEvent& other) = delete;
	IocpEvent& operator=(IocpEvent&& other) = delete;
	~IocpEvent() = default;

	void Init();

public:
	EventType m_eventType;
	shared_ptr<IocpObject> m_owner;
};

/* ----------------------------
*		AcceptEvent
---------------------------- */

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::ACCEPT){}

public:
	shared_ptr<IocpSession>	m_session = nullptr;
};

/* ----------------------------
*		ConnectEvent
---------------------------- */

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::CONNECT) {}
};

/* ----------------------------
*		DisconnectEvent
---------------------------- */

class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::DISCONNECT) {}
};

/* ----------------------------
*		RecvEvent
---------------------------- */

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::RECV) {}
};

/* ----------------------------
*		SendEvent
---------------------------- */

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::SEND) {}

	// TEMP
	vector<BYTE> m_buffer;
};