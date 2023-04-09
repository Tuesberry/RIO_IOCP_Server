#include "DelayManager.h"

DelayManager gDelayMgr;

DelayManager::DelayManager()
	: m_updateDelayLock()
	, m_avgDelay(0)
	, m_delayCnt(0)
	, m_updateLoginLock()
	, m_avgLoginDelay(0)
	, m_loginCnt(0)
	, m_updateProcessLock()
	, m_avgProcessTime(0)
	, m_processCnt(0)
	, m_updateCnt(0)
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

void DelayManager::UpdateAvgProcessTime(int processTime, int prevProcessTime)
{
	lock_guard<mutex> lock(m_updateProcessLock);
	m_avgProcessTime = static_cast<long double>(m_avgProcessTime * m_processCnt - prevProcessTime + processTime) / m_processCnt;
}

void DelayManager::AddNewInAvgProcessTime(int processTime)
{
	lock_guard<mutex> lock(m_updateProcessLock);
	m_avgProcessTime = static_cast<long double>(m_avgProcessTime * m_processCnt + processTime) / (static_cast<long double>(m_processCnt) + 1);
	m_processCnt++;
}

void DelayManager::UpdateLoginDelay(int delay)
{
	lock_guard<mutex> lock(m_updateLoginLock);
	m_avgLoginDelay = static_cast<long double>(m_avgLoginDelay * m_loginCnt + delay) / (static_cast<long double>(m_loginCnt) + 1);
	m_loginCnt++;
}
