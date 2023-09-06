#pragma once

#include "Common.h"

#include "NetBuffer.h"

class IocpObject;
class IocpSession;

/* --------------------------------------------------------
*	class:		IOCP_IO_TYPE
*	Summary:	Io type used by iocp
-------------------------------------------------------- */
enum class IOCP_IO_TYPE
{
	ACCEPT,
	CONNECT,
	DISCONNECT,
	RECV,
	SEND
};

/* --------------------------------------------------------
*	class:		IocpEvent
*	Summary:	Inherit the OVERLAPPED structure.
*				It contains pointer about the owner.
-------------------------------------------------------- */
class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent() = delete;
	IocpEvent(IOCP_IO_TYPE type);
	IocpEvent(const IocpEvent& other) = delete;
	IocpEvent(IocpEvent&& other) = delete;
	IocpEvent& operator=(const IocpEvent& other) = delete;
	IocpEvent& operator=(IocpEvent&& other) = delete;
	~IocpEvent() = default;

	void Init();

public:
	IOCP_IO_TYPE m_eventType;
	shared_ptr<IocpObject> m_owner;
};

/* --------------------------------------------------------
*	class:		AcceptEvent
*	Summary:	Event for accept request
				It contains accept socket session
-------------------------------------------------------- */
class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() :IocpEvent(IOCP_IO_TYPE::ACCEPT) {}

	AcceptEvent(const AcceptEvent& other) = delete;
	AcceptEvent(AcceptEvent&& other) = delete;
	AcceptEvent& operator=(const AcceptEvent& other) = delete;
	AcceptEvent& operator=(AcceptEvent&& other) = delete;
	~AcceptEvent() = default;

public:
	// accept socket session
	shared_ptr<IocpSession> m_session = nullptr;
};

/* --------------------------------------------------------
*	class:		ConnectEvent
*	Summary:	Event for connect request
-------------------------------------------------------- */
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() :IocpEvent(IOCP_IO_TYPE::CONNECT) {}

	ConnectEvent(const ConnectEvent& other) = delete;
	ConnectEvent(ConnectEvent&& other) = delete;
	ConnectEvent& operator=(const ConnectEvent& other) = delete;
	ConnectEvent& operator=(ConnectEvent&& other) = delete;
	~ConnectEvent() = default;
};

/* --------------------------------------------------------
*	class:		DisconnectEvent
*	Summary:	Event for disconnect request
-------------------------------------------------------- */
class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() :IocpEvent(IOCP_IO_TYPE::DISCONNECT) {}

	DisconnectEvent(const DisconnectEvent& other) = delete;
	DisconnectEvent(DisconnectEvent&& other) = delete;
	DisconnectEvent& operator=(const DisconnectEvent& other) = delete;
	DisconnectEvent& operator=(DisconnectEvent&& other) = delete;
	~DisconnectEvent() = default;
};

/* --------------------------------------------------------
*	class:		RecvEvent
*	Summary:	Event for recv request
-------------------------------------------------------- */
class RecvEvent : public IocpEvent
{
public:
	RecvEvent() :IocpEvent(IOCP_IO_TYPE::RECV) {}

	RecvEvent(const RecvEvent& other) = delete;
	RecvEvent(RecvEvent&& other) = delete;
	RecvEvent& operator=(const RecvEvent& other) = delete;
	RecvEvent& operator=(RecvEvent&& other) = delete;
	~RecvEvent() = default;
};

/* --------------------------------------------------------
*	class:		SendEvent
*	Summary:	Event for send request
-------------------------------------------------------- */
class SendEvent : public IocpEvent
{
public:
	SendEvent() :IocpEvent(IOCP_IO_TYPE::SEND) {}

	SendEvent(const SendEvent& other) = delete;
	SendEvent(SendEvent&& other) = delete;
	SendEvent& operator=(const SendEvent& other) = delete;
	SendEvent& operator=(SendEvent&& other) = delete;
	~SendEvent() = default;
public:
	vector<shared_ptr<NetBuffer>> m_sendBuffer;
};