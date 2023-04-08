#include "DelayManager.h"

DelayManager gDelayMgr;

DelayManager::DelayManager()
	: m_avgDelay(0)
	, m_delayCnt(0)
{
}

void DelayManager::UpdateAvgDelay(int delay, int prevDelay)
{
	lock_guard<mutex> lock(m_updateDelayLock);
	m_avgDelay = static_cast<long double>(m_avgDelay * m_delayCnt - prevDelay + delay) / m_delayCnt;
}

void DelayManager::AddNewInAvgDelay(int delay)
{
	lock_guard<mutex> lock(m_updateDelayLock);
	m_avgDelay = static_cast<long double>(m_avgDelay * m_delayCnt + delay) / (static_cast<long double>(m_delayCnt) + 1);
	m_delayCnt++;
}
