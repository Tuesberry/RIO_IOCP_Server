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
	m_avgDelay = ((long double)m_dataCnt - 1) / (long double)m_dataCnt * m_avgDelay + (long double)1 / (long double)m_dataCnt * (long double)delay;
}

/* --------------------------------------------------------
*	Method:		DelayChecker::GetAvgDelay
*	Summary:	get average delay
-------------------------------------------------------- */
long double DelayChecker::GetAvgDelay()
{
	lock_guard<mutex> lock(m_updateLock);
	return m_avgDelay;
}

/* --------------------------------------------------------
*	Method:		DelayChecker::Reset
*	Summary:	reset delay checker
-------------------------------------------------------- */
void DelayChecker::Reset()
{
	m_dataCnt = 0;
	m_avgDelay = 0;
}

/* --------------------------------------------------------
*	Method:		DelayManager::Reset
*	Summary:	reset delay checker
-------------------------------------------------------- */
void DelayManager::Reset()
{
	m_avgProcessDelay.Reset();
	m_avgLoginDelay.Reset();
	m_avgReceivingDelay.Reset();
	m_avgSendingDelay.Reset();
	m_avgSendRecvDelay.Reset();
	m_avgUpdatePosDelay.Reset();
	m_avgSynchronizeMoveDelay.Reset();

	m_sendCnt.store(0);
	m_recvCnt.store(0);
}
