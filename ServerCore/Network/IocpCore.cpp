#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
	:m_iocpHandle(NULL)
{
	if (CreateIocpHandle() == false)
		HandleError("Create Iocp Completion Port");
}

IocpCore::~IocpCore()
{
	::CloseHandle(m_iocpHandle);
}

bool IocpCore::Register(shared_ptr<IocpObject> iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), m_iocpHandle, 0, 0);
}

bool IocpCore::ExecuteTask(unsigned int timeOut)
{
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	// GetQueuedCompletionStatus
	BOOL retVal = ::GetQueuedCompletionStatus(m_iocpHandle, &bytesTransferred, &key, (LPOVERLAPPED*)&iocpEvent, timeOut);
	
	if (retVal == TRUE)
	{
		shared_ptr<IocpObject> iocpObject = iocpEvent->m_owner;
		iocpObject->ExecuteTask(iocpEvent, bytesTransferred);
	}
	else
	{
		if (::WSAGetLastError() == WAIT_TIMEOUT)
		{
			// timeout error
			return false;
		}
		// error check?
		shared_ptr<IocpObject> iocpObject = iocpEvent->m_owner;
		iocpObject->ExecuteTask(iocpEvent, bytesTransferred);
	}

	return true;
}

bool IocpCore::CreateIocpHandle()
{
	m_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_iocpHandle == NULL)
	{
		return false;
	}
	return true;
}
