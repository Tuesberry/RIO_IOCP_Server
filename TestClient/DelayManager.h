#pragma once
#include "CoreCommon.h"

class DelayManager
{
public:
	DelayManager();
	DelayManager(const DelayManager& other) = delete;
	DelayManager(DelayManager&& other) = delete;
	DelayManager& operator=(const DelayManager& other) = delete;
	DelayManager& operator=(DelayManager&& other) = delete;
	~DelayManager() = default;

	void UpdateAvgDelay(int delay, int prevDelay);
	void AddNewInAvgDelay(int delay);

	long double GetAvgDelay() { return m_avgDelay; }
	int GetDelayCng() { return m_delayCnt; }

private:
	mutex m_updateDelayLock;

	long double m_avgDelay; 
	int m_delayCnt;
};

extern DelayManager gDelayMgr;