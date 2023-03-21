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

SOCKET SocketCore::Socket()
{
	return CreateSocket();
}

bool SocketCore::Close(SOCKET& socket)
{
	if (socket == INVALID_SOCKET)
		return false;

	::closesocket(socket);
	socket = INVALID_SOCKET;

	return true;
}

bool SocketCore::Bind(SOCKET socket, SockAddress sockAddr)
{
	if (socket == INVALID_SOCKET)
		return false;

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&sockAddr.GetSockAddr()), sockAddr.GetAddrSize());
}

bool SocketCore::BindAddrAny(SOCKET socket, unsigned short port)
{
	SOCKADDR_IN address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = ::htonl(INADDR_ANY);
	address.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&address), sizeof(address));
}

bool SocketCore::Listen(SOCKET socket)
{
	if (socket == INVALID_SOCKET)
		return false;

	return SOCKET_ERROR != ::listen(socket, SOMAXCONN);
}

SOCKET SocketCore::Accept(SOCKET socket)
{
	if (socket == INVALID_SOCKET)
		return INVALID_SOCKET;

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	return ::accept(socket, (SOCKADDR*)&clientAddr, &addrLen);
}

bool SocketCore::Connect(SOCKET socket, SockAddress& sockAddr)
{
	if (socket == INVALID_SOCKET)
		return false;	

	return SOCKET_ERROR != ::connect(socket, (SOCKADDR*)&sockAddr.GetSockAddr(), sockAddr.GetAddrSize());
}

int SocketCore::Send(SOCKET socket, char* buf, int size)
{
	if (socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	return ::send(socket, buf, size, 0);
}

int SocketCore::Recv(SOCKET socket, char* buf, int size)
{
	if (socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	return ::recv(socket, buf, size, 0);
}

bool SocketCore::SetNodelay(SOCKET socket, bool optVal)
{
	return SetSockOpt<int>(socket, IPPROTO_TCP, TCP_NODELAY, optVal);
}

bool SocketCore::SetLinger(SOCKET socket, bool optVal, int time)
{
	linger templinger;
	templinger.l_onoff = optVal;
	templinger.l_linger = time;

	return SetSockOpt<linger>(socket, SOL_SOCKET, SO_LINGER, templinger);
}

bool SocketCore::SetSendBufSiz(SOCKET socket, int size)
{
	return SetSockOpt<char>(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketCore::SetRecvBufSiz(SOCKET socket, int size)
{
	return SetSockOpt<char>(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketCore::SetReuseAddr(SOCKET socket, bool optVal)
{
	return SetSockOpt<int>(socket, SOL_SOCKET, SO_REUSEADDR, optVal);
}

bool SocketCore::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt<SOCKET>(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
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
bool SocketCore::SetSockOpt(SOCKET socket, int level, int optName, T optVal)
{
	if (socket == INVALID_SOCKET)
		return false;

	return SOCKET_ERROR != setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}
