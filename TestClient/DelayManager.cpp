#include "DelayManager.h"

DelayManager gDelayMgr;

/* --------------------------------------------------------
*	Method:		DelayChecker::UpdateAvgDelay
*	Summary:	update average delay 
				using average filter algorithm
*	Args:		int delay
*					used for average delay update
-------------------------------------------------------- */
void DelayChecker::UpdateAvgDelay(IN int delay)
{
	lock_guard<mutex> lock(m_updateLock);
	m_dataCnt++;
	m_avgDelay = (m_dataCnt - 1) / m_dataCnt * m_avgDelay + (long double)1 / m_dataCnt * delay;
}
