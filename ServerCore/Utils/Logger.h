#pragma once

#include "../Common.h"

enum class ELOG_LEVEL
{
	LOG_LEVEL_NONE,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFORM,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR
};

class Logger
{
public:
	Logger() = default;
	~Logger() = default;

	static ELOG_LEVEL s_loggerLevel;

};