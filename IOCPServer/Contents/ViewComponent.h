#pragma once

#include "../IOCPServerCommon.h"

#include "Thread/RWLock.h"

class ViewComponent
{
public:
	ViewComponent();
	~ViewComponent();

private:

private:
	RWLock m_lock;
	unordered_set<int> m_viewList;
};