#pragma once

#include "../DummyClientCommon.h"

/* --------------------------------------------------------
*	class:		DelayChecker
*	Summary:	update delay average
-------------------------------------------------------- */

class DelayChecker final
{
public:
	DelayChecker() = default;
	DelayChecker(const DelayChecker& other) = delete;
	DelayChecker(DelayChecker&& other) = delete;
	DelayChecker& operator=(const DelayChecker& other) = delete;
	DelayChecker& operator=(DelayChecker&& other) = delete;
	~DelayChecker() = default;

	void UpdateAvgDelay(IN int delay);
	long double GetAvgDelay();

	void Reset();

private:
	mutex m_updateLock;
	double m_avgDelay = 0;
	long int m_dataCnt = 0;
};

/* --------------------------------------------------------
*	class:		DelayManager
*	Summary:	DelayManager manages delay used for 
				testing server
-------------------------------------------------------- */

class DelayManager final
{
public:
	DelayManager() = default;
	DelayManager(const DelayManager& other) = delete;
	DelayManager(DelayManager&& other) = delete;
	DelayManager& operator=(const DelayManager& other) = delete;
	DelayManager& operator=(DelayManager&& other) = delete;
	~DelayManager() = default;

	void Reset();
public:
	DelayChecker m_avgProcessDelay;
	DelayChecker m_avgLoginDelay;
	DelayChecker m_avgReceivingDelay;
	DelayChecker m_avgSendingDelay;
	DelayChecker m_avgSendRecvDelay;

	atomic<int> m_sendCnt = 0;
	atomic<int> m_recvCnt = 0;
};

extern DelayManager gDelayMgr;