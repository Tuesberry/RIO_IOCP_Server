#pragma once
#include "Common.h"

/* ----------------------------
*		IocpObject
---------------------------- */

class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void ExecuteTask(class IocpEvent* iocpEvent, int bytesTransferred = 0) abstract;
};

/* ----------------------------
*		IocpCore
---------------------------- */

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
	bool ExecuteTask(unsigned int timeOut = INFINITE);

private:
	bool CreateIocpHandle();

private:
	HANDLE m_iocpHandle;
};