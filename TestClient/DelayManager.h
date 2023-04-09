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

	void UpdateAvgProcessTime(int processTime, int prevProcessTime);
	void AddNewInAvgProcessTime(int processTime);

	void UpdateLoginDelay(int delay);

	long double GetAvgDelay() { return m_avgDelay; }
	int GetDelayCnt() { return m_delayCnt; }
	long double GetAvgLoginDelay() { return m_avgLoginDelay; }
	int GetLoginCnt() { return m_loginCnt; }
	long double GetAvgProcessTime() { return m_avgProcessTime; }
	int GetProcessCnt() { return m_processCnt; }

public:
	long long int m_updateCnt;

private:
	mutex m_updateDelayLock;
	long double m_avgDelay; 
	int m_delayCnt;

	mutex m_updateLoginLock;
	long double m_avgLoginDelay;
	int m_loginCnt;

	mutex m_updateProcessLock;
	long double m_avgProcessTime;
	int m_processCnt;
};

extern DelayManager gDelayMgr;