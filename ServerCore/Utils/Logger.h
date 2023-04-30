#pragma once

#include "Common.h"

#include <fstream>

enum class LogLevel
{
	EXCEPTION,
	WARN,
	LOG
};

class Logger
{
public:
	Logger(string fileName);

	Logger() = delete;
	Logger(const Logger& other) = delete;
	Logger(Logger&& other) = delete;
	Logger& operator=(const Logger& other) = delete;
	Logger& operator=(Logger&& other) = delete;
	~Logger();

	void Log(string msg);
	void WriteLog();

private:
	mutex m_lock;
	queue<string> m_queue;

	ofstream m_logFile;
};