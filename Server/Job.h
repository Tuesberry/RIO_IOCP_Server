#pragma once

#include "Common.h"

/* --------------------------------------------------------
*	class:		Job
*	Summary:	callback function wrapper class
-------------------------------------------------------- */
class Job
{
public:
	// callback function
	Job(std::function<void()> callback):m_callback(callback){}

	// class member callback function
	template<typename T, typename Ret, typename...Args>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		m_callback = [owner, memFunc, args...]()
		{
			(owner.get()->*memFunc)(args...);
		};
	}

	void Execute()
	{
		m_callback();
	}

private:
	std::function<void()> m_callback;
};