#pragma once

#include "Common.h"

#include "Memory/ObjectPool.h"

class RioSession;

/* --------------------------------------------------------
*	class:		RIO_IO_TYPE
*	Summary:	IO Type used by RIO
-------------------------------------------------------- */
enum class RIO_IO_TYPE
{
	RECV,
	SEND
};

/* --------------------------------------------------------
*	class:		RioEvent
*	Summary:	Inherit the Poolable<RioEvent> and RIO_BUF.
*				It contains pointer about the owner.
-------------------------------------------------------- */
class RioEvent : public Poolable<RioEvent>, public RIO_BUF
{
public:
	RioEvent(RIO_IO_TYPE ioType);

	RioEvent() = delete;
	RioEvent(const RioEvent& other) = delete;
	RioEvent(RioEvent&& other) = delete;
	RioEvent& operator=(const RioEvent& other) = delete;
	RioEvent& operator=(RioEvent&& other) = delete;
	~RioEvent() = default;
public:
	shared_ptr<RioSession> m_owner;
	RIO_IO_TYPE m_ioType;
};

/* --------------------------------------------------------
*	class:		RioSendEvent
*	Summary:	Event for send request
-------------------------------------------------------- */
class RioSendEvent : public RioEvent
{
public:
	RioSendEvent():RioEvent(RIO_IO_TYPE::SEND){}
	RioSendEvent(const RioSendEvent& other) = delete;
	RioSendEvent(RioSendEvent&& other) = delete;
	RioSendEvent& operator=(const RioSendEvent& other) = delete;
	RioSendEvent& operator=(RioSendEvent&& other) = delete;
	~RioSendEvent() = default;
};

/* --------------------------------------------------------
*	class:		RioRecvEvent
*	Summary:	Event for recv request
-------------------------------------------------------- */
class RioRecvEvent : public RioEvent
{
public:
	RioRecvEvent():RioEvent(RIO_IO_TYPE::RECV) {}
	RioRecvEvent(const RioRecvEvent& other) = delete;
	RioRecvEvent(RioRecvEvent&& other) = delete;
	RioRecvEvent& operator=(const RioRecvEvent& other) = delete;
	RioRecvEvent& operator=(RioRecvEvent&& other) = delete;
	~RioRecvEvent() = default;
};