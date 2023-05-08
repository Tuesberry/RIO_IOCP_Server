#include "Logger.h"

void Logger::Log(string log)
{
	GetInstance().WriteLog(log);
}

bool Logger::SetFileLog(string path)
{
	// get instance
	Logger& logInstance = GetInstance();

	// set provider
	logInstance.m_logProvider = LogProvider::File;

	// set path
	string time = GetCurrentTimeStr();
	logInstance.m_filePath = path + " " + time;

	// open file
	return logInstance.OpenFile();
}

void Logger::SetConsoleLog()
{
	Logger& logInstance = GetInstance();
	logInstance.m_logProvider = LogProvider::Console;
}

Logger::Logger()
	: m_file(nullptr)
	, m_filePath()
	, m_lock()
	, m_logProvider(LogProvider::Console)
{
}

Logger::~Logger()
{
	CloseFile();
}

bool Logger::OpenFile()
{
	CloseFile();

	m_file = fopen(m_filePath.c_str(), "a");

	if (m_file == nullptr)
	{
		return false;
	}

	return true;
}

void Logger::CloseFile()
{
	if (m_file)
	{
		fclose(m_file);
		m_file = nullptr;
	}
}

void Logger::WriteLog(string log)
{
	lock_guard<mutex> lock(m_lock);
	if (m_logProvider == LogProvider::File && m_file)
	{
		fprintf(m_file, log.c_str());
		fprintf(m_file, "\n");
	}
	else if (m_logProvider == LogProvider::Console)
	{
		printf(GetCurrentTimeStr().c_str());
		printf(log.c_str());
		printf("\n");
	}
}

string Logger::GetCurrentTimeStr()
{
	time_t timer = time(NULL);
	tm* t = localtime(&timer);
	return to_string(t->tm_mon) + "." + to_string(t->tm_mday) + "." + to_string(t->tm_hour) + "." + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + " ";
}

