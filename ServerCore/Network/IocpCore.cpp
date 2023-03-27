#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
	: m_iocpHandle(nullptr)
{
	CreateIocpHandle();
}

IocpCore::~IocpCore()
{
	::CloseHandle(m_iocpHandle);
}

bool IocpCore::Register(shared_ptr<IocpObject> iocpObject)
{
	if (::CreateIoCompletionPort(iocpObject->GetHandle(), m_iocpHandle, /*key*/0, 0) == NULL)
		return false;

	return true;
}

bool IocpCore::Dispatch(unsigned int timeOutMs)
{
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	BOOL retVal = ::GetQueuedCompletionStatus(m_iocpHandle, &bytesTransferred, &key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeOutMs);

	if (retVal == TRUE)
	{
		shared_ptr<IocpObject> iocpObject = iocpEvent->m_owner;
		iocpObject->Dispatch(iocpEvent, bytesTransferred);
	}
	else
	{
		int errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			HandleError("WAIT_TIMEOUT");
			return false;
		default:
			shared_ptr<IocpObject> iocpObject = iocpEvent->m_owner;
			iocpObject->Dispatch(iocpEvent, bytesTransferred);
			break;
		}
	}

	return true;
}

void IocpCore::CreateIocpHandle()
{
	m_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_iocpHandle == NULL)
	{
		HandleError("CreateIocpHandle");
	}
}


