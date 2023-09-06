#include "IocpEvent.h"

/* --------------------------------------------------------
*	Method:		IocpEvent::IocpEvent
*	Summary:	constructor
*	Args:		IOCP_IO_TYPE type
*					io request type
-------------------------------------------------------- */
IocpEvent::IocpEvent(IOCP_IO_TYPE type)
	: m_eventType(type)
	, m_owner(nullptr)
{
	Init();
}

/* --------------------------------------------------------
*	Method:		IocpEvent::Init
*	Summary:	initialize overlapped structure
-------------------------------------------------------- */
void IocpEvent::Init()
{
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
}
