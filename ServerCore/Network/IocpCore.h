#pragma once

#include "Common.h"

class IocpEvent;

/* --------------------------------------------------------
*	class:		IocpObject
*	Summary:	Objects managed by iocp core 
-------------------------------------------------------- */
class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(IocpEvent* iocpEvent, int bytesTransferred = 0) abstract;
};

/* --------------------------------------------------------
*	class:		IocpCore
*	Summary:	The core of IOCP.
				It's the class responsible for completion port
-------------------------------------------------------- */
class IocpCore
{
public:
	IocpCore();
	IocpCore(const IocpCore& other) = delete;
	IocpCore(IocpCore&& other) = delete;
	IocpCore& operator=(const IocpCore& other) = delete;
	IocpCore& operator=(IocpCore&& other) = delete;
	~IocpCore();

	HANDLE GetHandle() { return m_iocpHandle; }

	bool Register(shared_ptr<IocpObject> iocpObject);
	bool Dispatch(unsigned int timeOutMs = INFINITE);

private:
	void CreateIocpHandle();

private:
	HANDLE m_iocpHandle;
};