#pragma once

#include "Common.h"

#include <cstdio>
#pragma warning(disable : 4996)

/* --------------------------------------------------------
*	class:		Logger
*	Summary:	Logger for multithread
-------------------------------------------------------- */
class Logger
{
public:
	enum class LogProvider
	{
		Console,
		File,
	};

	static void Log(string log);

	static bool SetFileLog(string path);
	static void SetConsoleLog();

	static string GetCurrentTimeStr();

private:
	static Logger& GetInstance()
	{
		static Logger instance;
		return instance;
	}

	Logger();
	Logger(const Logger& other) = delete;
	Logger(Logger&& other) = delete;
	Logger& operator=(const Logger& other) = delete;
	Logger& operator=(Logger&& other) = delete;
	~Logger();

	bool OpenFile();
	void CloseFile();

	void WriteLog(string log);

	
private:
	LogProvider m_logProvider;
	mutex m_lock;
	string m_filePath;
	FILE* m_file;
};