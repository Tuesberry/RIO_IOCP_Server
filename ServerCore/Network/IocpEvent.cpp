#include "IocpEvent.h"

IocpEvent::IocpEvent(EventType type)
	:m_eventType(type)
	,m_owner(nullptr)
{
	Init();
}

void IocpEvent::Init()
{
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
}
