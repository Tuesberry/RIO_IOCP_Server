#include "Logger.h"

Logger::Logger(string fileName)
{
	m_logFile.open(fileName);
}

Logger::~Logger()
{
	m_logFile.close();
}

void Logger::Log(string msg)
{
	lock_guard<mutex> lock(m_lock);
	//m_queue.push(msg);
	m_logFile << msg << endl;
}

void Logger::WriteLog()
{
	lock_guard<mutex> lock(m_lock);
	if (m_queue.empty())
		return;
	
	string msg = m_queue.front();
	m_queue.pop();

	m_logFile << msg << endl;
}
