#include "RioEvent.h"
#include "RioSession.h"

RioEvent::RioEvent(RIO_IO_TYPE ioType)
	: m_owner(nullptr)
	, m_ioType(ioType)
{
}
