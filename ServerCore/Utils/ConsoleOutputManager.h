#pragma once
#include "Common.h"

class ConsoleOutputManager
{
public:
	ConsoleOutputManager();
	ConsoleOutputManager(const ConsoleOutputManager& other) = delete;
	ConsoleOutputManager(ConsoleOutputManager&& other) = delete;
	ConsoleOutputManager& operator=(const ConsoleOutputManager& other) = delete;
	ConsoleOutputManager& operator=(ConsoleOutputManager&& other) = delete;
	~ConsoleOutputManager() = default;

	template<typename T>
	ConsoleOutputManager& operator<<(T& src);

	template<typename T>
	ConsoleOutputManager& operator<<(const T& src);

	ConsoleOutputManager& operator<<(std::ostream& (*f)(std::ostream&));
	ConsoleOutputManager& operator<<(std::ostream& (*f)(std::ios&));
	ConsoleOutputManager& operator<<(std::ostream& (*f)(std::ios_base&));

private:
	mutex m_outputLock;
};

template<typename T>
ConsoleOutputManager& ConsoleOutputManager::operator<<(T& src)
{
	lock_guard<mutex> outputLock(m_outputLock);
	cout << src << endl;
	return *this;
}

template<typename T>
ConsoleOutputManager& ConsoleOutputManager::operator<<(const T& src)
{
	lock_guard<mutex> outputLock(m_outputLock);
	cout << src << endl;
	return *this;
}