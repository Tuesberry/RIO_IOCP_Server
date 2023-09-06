#include "RioEvent.h"
#include "RioSession.h"
#include "RioCore.h"

/* --------------------------------------------------------
*	Method:		RioEvent::RioEvent
*	Summary:	Constructor
-------------------------------------------------------- */
RioEvent::RioEvent(RIO_IO_TYPE ioType)
	: m_owner(nullptr)
	, m_ioType(ioType)
{
}
