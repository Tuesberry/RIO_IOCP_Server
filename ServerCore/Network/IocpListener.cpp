#include "IocpListener.h"
#include "SocketCore.h"
#include "SockAddress.h"

IocpListener::IocpListener()
	: m_listener(INVALID_SOCKET)
{
}

IocpListener::~IocpListener()
{
	CloseSocket();
}

bool IocpListener::StartAccept(unsigned int portNum)
{
	// listen socket
	m_listener = SocketCore::Socket();
	if (m_listener == INVALID_SOCKET)
	{
		HandleError("listen socket create");
		return false;
	}

	// bind
	if (SocketCore::BindAddrAny(m_listener, portNum) == false)
	{
		HandleError("listen socket bind");
		return false;
	}

	// listen
	if (SocketCore::Listen(m_listener) == false)
	{
		HandleError("listen");
		return false;
	}

	return true;
}

void IocpListener::CloseSocket()
{
	if (SocketCore::Close(m_listener) == false)
	{
		HandleError("IocpListener::CloseSocket");
	}
}

void IocpListener::RegisterAccept(AcceptEvent* acceptEvent)
{

}

void IocpListener::ProcessAccept(AcceptEvent* acceptEvent)
{

}

HANDLE IocpListener::GetHandle()
{
	return reinterpret_cast<HANDLE>(m_listener);
}

void IocpListener::Dispatch(IocpEvent* iocpEvent, int bytesTransferred)
{
}
