#include "RioEvent.h"
#include "RioSession.h"
#include "RioCore.h"

RioEvent::RioEvent(RIO_IO_TYPE ioType)
	: m_owner(nullptr)
	, m_ioType(ioType)
{
}
