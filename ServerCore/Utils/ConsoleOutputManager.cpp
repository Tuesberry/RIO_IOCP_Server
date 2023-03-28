#include "ConsoleOutputManager.h"

ConsoleOutputManager::ConsoleOutputManager()
	: m_outputLock()
{
}

ConsoleOutputManager& ConsoleOutputManager::operator<<(std::ostream& (*f)(std::ostream&))
{
	lock_guard<mutex> outputLock(m_outputLock);
	f(std::cout);
	return *this;
}

ConsoleOutputManager& ConsoleOutputManager::operator<<(std::ostream& (*f)(std::ios&))
{
	lock_guard<mutex> outputLock(m_outputLock);
	f(std::cout);
	return *this;
}

ConsoleOutputManager& ConsoleOutputManager::operator<<(std::ostream& (*f)(std::ios_base&))
{
	lock_guard<mutex> outputLock(m_outputLock);
	f(std::cout);
	return *this;
}
