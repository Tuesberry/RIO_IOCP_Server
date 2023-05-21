#pragma once
#include "Common.h"

class RioSession;
class RioCore;

/* ----------------------------
*		RioCQEvent
---------------------------- */

class RioCQEvent : public OVERLAPPED
{
public:
	RioCQEvent() = default;
	RioCQEvent(const RioCQEvent& other) = delete;
	RioCQEvent(RioCQEvent&& other) = delete;
	RioCQEvent& operator=(const RioCQEvent& other) = delete;
	RioCQEvent& operator=(RioCQEvent&& other) = delete;
	~RioCQEvent() = default;

public:
	shared_ptr<RioCore> m_ownerCore;
};

/* ----------------------------
*		RIO_IO_TYPE
---------------------------- */

enum class RIO_IO_TYPE
{
	RECV,
	SEND
};

/* ----------------------------
*		RioEvent
---------------------------- */

class RioEvent : public RIO_BUF
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

/* ----------------------------
*		RioSendEvent
---------------------------- */

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

/* ----------------------------
*		RioRecvEvent
---------------------------- */

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