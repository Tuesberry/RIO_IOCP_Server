#include "IocpCore.h"
#include "IocpEvent.h"

/* --------------------------------------------------------
*	Method:		IocpCore::IocpCore
*	Summary:	constructor
-------------------------------------------------------- */
IocpCore::IocpCore()
	: m_iocpHandle(nullptr)
{
	CreateIocpHandle();
}

/* --------------------------------------------------------
*	Method:		IocpCore::~IocpCore
*	Summary:	destructor
-------------------------------------------------------- */
IocpCore::~IocpCore()
{
	::CloseHandle(m_iocpHandle);
}

/* --------------------------------------------------------
*	Method:		IocpCore::Register
*	Summary:	register the iocpObject(client socket) 
				with completion port
*	Args:		shared_ptr<IocpObject> iocpObject
*					iocpObject to register in completion port
-------------------------------------------------------- */
bool IocpCore::Register(shared_ptr<IocpObject> iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), m_iocpHandle, /*key*/0, 0) != NULL;
}

/* --------------------------------------------------------
*	Method:		IocpCore::Dispatch
*	Summary:	Worker Thread check and dispatch work 
				using GetQueuedCompletionStatus
*	Args:		unsigned int timeOutMs
*					set wait time limit
-------------------------------------------------------- */
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
			break;
		default:
			shared_ptr<IocpObject> iocpObject = iocpEvent->m_owner;
			iocpObject->Dispatch(iocpEvent, bytesTransferred);
			break;
		}
	}

	return true;
}

/* --------------------------------------------------------
*	Method:		IocpCore::CreateIocpHandle
*	Summary:	create completion port
-------------------------------------------------------- */
void IocpCore::CreateIocpHandle()
{
	m_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_iocpHandle == NULL)
	{
		HandleError("CreateIocpHandle");
	}
}


