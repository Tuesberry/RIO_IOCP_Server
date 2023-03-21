#include "SocketCore.h"

LPFN_CONNECTEX SocketCore::ConnectEx = nullptr;
LPFN_DISCONNECTEX SocketCore::DisconnectEx = nullptr;
LPFN_ACCEPTEX SocketCore::AcceptEx = nullptr;

SocketCore::SocketCore()
	: m_socket(INVALID_SOCKET)
{
}

bool SocketCore::Init()
{
	//Init Winsock
	WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	//Bind Socket Functions
	if (ConnectEx != nullptr && AcceptEx != nullptr && DisconnectEx != nullptr)
		return true;

	SOCKET tempSocket = CreateSocket();
	if (BindWindowsFunction(tempSocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)) == false)
		return false;
	if (BindWindowsFunction(tempSocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)) == false)
		return false;
	if (BindWindowsFunction(tempSocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)) == false)
		return false;

	return true;
}

bool SocketCore::Clear()
{
	if (::WSACleanup() != 0)
		return false;

	return true;
}

bool SocketCore::Socket()
{
	m_socket = CreateSocket();
	if (m_socket == INVALID_SOCKET)
		return false;
	return true;
}

bool SocketCore::Close()
{
	if (m_socket == INVALID_SOCKET)
		return false;

	::closesocket(m_socket);
	m_socket = INVALID_SOCKET;

	return true;
}

bool SocketCore::Bind(SockAddress& sockAddr)
{
	if (m_socket == INVALID_SOCKET)
		return false;

	if (::bind(m_socket, (SOCKADDR*)&sockAddr.GetSockAddr(), sockAddr.GetAddrSize()) == SOCKET_ERROR)
		return false;

	return true;
}

bool SocketCore::Listen()
{
	if (m_socket == INVALID_SOCKET)
		return false;

	if (::listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
		return false;

	return true;
}

SOCKET SocketCore::Accept()
{
	if (m_socket == INVALID_SOCKET)
		return INVALID_SOCKET;

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	return ::accept(m_socket, (SOCKADDR*)&clientAddr, &addrLen);
}

bool SocketCore::Connect(SockAddress& sockAddr)
{
	if (m_socket == INVALID_SOCKET)
		return false;	

	if (::connect(m_socket, (SOCKADDR*)&sockAddr.GetSockAddr(), sockAddr.GetAddrSize()) == SOCKET_ERROR)
		return false;

	return true;
}

int SocketCore::Send(char* buf, int size)
{
	if (m_socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	return ::send(m_socket, buf, size, 0);
}

int SocketCore::Recv(char* buf, int size)
{
	if (m_socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	return ::recv(m_socket, buf, size, 0);
}

void SocketCore::SetNodelay(bool optVal)
{
	SetSockOpt<int>(IPPROTO_TCP, TCP_NODELAY, optVal);
}

void SocketCore::SetLinger(bool optVal, int time)
{
	linger templinger;
	templinger.l_onoff = optVal;
	templinger.l_linger = time;

	SetSockOpt<linger>(SOL_SOCKET, SO_LINGER, templinger);
}

void SocketCore::SetSendBufSiz(int size)
{
	SetSockOpt<char>(SOL_SOCKET, SO_SNDBUF, size);
}

void SocketCore::SetRecvBufSiz(int size)
{
	SetSockOpt<char>(SOL_SOCKET, SO_RCVBUF, size);
}

SOCKET SocketCore::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool SocketCore::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT & bytes, NULL, NULL);
}

template<typename T>
void SocketCore::SetSockOpt(int level, int optName, T optVal)
{
	if (m_socket == INVALID_SOCKET)
		return;

	setsockopt(m_socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}
