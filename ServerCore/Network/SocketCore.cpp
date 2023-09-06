#include "SocketCore.h"

LPFN_CONNECTEX SocketCore::ConnectEx = nullptr;
LPFN_DISCONNECTEX SocketCore::DisconnectEx = nullptr;
LPFN_ACCEPTEX SocketCore::AcceptEx = nullptr;
RIO_EXTENSION_FUNCTION_TABLE SocketCore::RIO = { 0, };

/* -----------------------------------------------------------------
*	Method:		SocketCore::Init
*	Summary:	Initialize the socket and bind socket functions
*				such as ConnectEx, AcceptEx and RioFunctionTables.
*				It must be called at the beginning of program start.
----------------------------------------------------------------- */
bool SocketCore::Init()
{
	//Init Winsock
	WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return false;
	}

	//Bind Socket Functions
	if (ConnectEx != nullptr && AcceptEx != nullptr && DisconnectEx != nullptr)
	{
		return true;
	}

	SOCKET tempSocket = CreateSocket();
	if (!BindWindowsFunction(tempSocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)))
	{
		return false;
	}
	if (!BindWindowsFunction(tempSocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)))
	{
		return false;
	}
	if (!BindWindowsFunction(tempSocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)))
	{
		return false;
	}
	if (!BindRioFunctionTable(tempSocket, WSAID_MULTIPLE_RIO, RIO))
	{
		return false;
	}

	return true;
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Clear
*	Summary:	Terminate WinSocket
----------------------------------------------------------------- */
bool SocketCore::Clear()
{
	if (::WSACleanup() != 0)
	{
		return false;
	}

	return true;
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Socket
*	Summary:	Create and return overlapped socket
----------------------------------------------------------------- */
SOCKET SocketCore::Socket()
{
	return CreateSocket();
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::RioSocket
*	Summary:	Create and return socket used in RIO
----------------------------------------------------------------- */
SOCKET SocketCore::RioSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO | WSA_FLAG_OVERLAPPED);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Close
*	Summary:	Terminate socket
----------------------------------------------------------------- */
bool SocketCore::Close(SOCKET& socket)
{
	if (socket == INVALID_SOCKET)
	{
		return false;
	}

	::closesocket(socket);
	socket = INVALID_SOCKET;

	return true;
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Bind
*	Summary:	Bind socket to address
----------------------------------------------------------------- */
bool SocketCore::Bind(SOCKET socket, SockAddress sockAddr)
{
	if (socket == INVALID_SOCKET)
	{
		return false;
	}

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&sockAddr.GetSockAddr()), sockAddr.GetAddrSize());
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Bind
*	Summary:	Bind socket using INADDR_ANY
----------------------------------------------------------------- */
bool SocketCore::BindAddrAny(SOCKET socket, unsigned short port)
{
	SOCKADDR_IN address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = ::htonl(INADDR_ANY);
	address.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&address), sizeof(address));
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Listen
*	Summary:	makes the socket in a state 
*				waiting for a connection request
----------------------------------------------------------------- */
bool SocketCore::Listen(SOCKET socket)
{
	if (socket == INVALID_SOCKET)
	{
		return false;
	}

	return SOCKET_ERROR != ::listen(socket, SOMAXCONN);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Accept
*	Summary:	accept the connection request
----------------------------------------------------------------- */
SOCKET SocketCore::Accept(SOCKET socket)
{
	if (socket == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	return ::accept(socket, (SOCKADDR*)&clientAddr, &addrLen);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Accept
*	Summary:	accept connection request
----------------------------------------------------------------- */
bool SocketCore::Connect(SOCKET socket, SockAddress& sockAddr)
{
	if (socket == INVALID_SOCKET)
	{
		return false;
	}

	return SOCKET_ERROR != ::connect(socket, (SOCKADDR*)&sockAddr.GetSockAddr(), sockAddr.GetAddrSize());
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Send
* 	Summary:	send data(byte) 
*	Args:		SOCKET socket
*					socket where to send data
*				char* buf
*					data to send
*				int size
*					data size(byte)
----------------------------------------------------------------- */
int SocketCore::Send(SOCKET socket, char* buf, int size)
{
	if (socket == INVALID_SOCKET)
	{
		return SOCKET_ERROR;
	}

	return ::send(socket, buf, size, 0);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::Recv
* 	Summary:	recv data from server
*	Args:		SOCKET socket
*					connected socket 
*				char* buf
*					buffer to receive data
*				int size
*					data size(byte)
----------------------------------------------------------------- */
int SocketCore::Recv(SOCKET socket, char* buf, int size)
{
	if (socket == INVALID_SOCKET)
	{
		return SOCKET_ERROR;
	}

	return ::recv(socket, buf, size, 0);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::SetNodelay
* 	Summary:	select whether to use the Nagle algorithm
*	Args:		SOCKET socket
*					target socket
*				bool optVal
*					whether or not to use
----------------------------------------------------------------- */
bool SocketCore::SetNodelay(SOCKET socket, bool optVal)
{
	return SetSockOpt<int>(socket, IPPROTO_TCP, TCP_NODELAY, optVal);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::SetLinger
* 	Summary:	select whether to use SO_LINGER option
*	Args:		SOCKET socket
*					target socket
*				bool optVal
*					whether or not to use
*				int time
*					waiting time
----------------------------------------------------------------- */
bool SocketCore::SetLinger(SOCKET socket, bool optVal, int time)
{
	linger templinger;
	templinger.l_onoff = optVal;
	templinger.l_linger = time;

	return SetSockOpt<linger>(socket, SOL_SOCKET, SO_LINGER, templinger);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::SetSendBufSize
* 	Summary:	adjust send buffer size using SO_SNDBUF
*	Args:		SOCKET socket
*					target socket
*				int size
*					buffer size
----------------------------------------------------------------- */
bool SocketCore::SetSendBufSize(SOCKET socket, int size)
{
	return SetSockOpt<char>(socket, SOL_SOCKET, SO_SNDBUF, size);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::SetRecvBufSize
* 	Summary:	adjust recv buffer size using SO_RCVBUF
*	Args:		SOCKET socket
*					target socket
*				int size
*					buffer size
----------------------------------------------------------------- */
bool SocketCore::SetRecvBufSize(SOCKET socket, int size)
{
	return SetSockOpt<char>(socket, SOL_SOCKET, SO_RCVBUF, size);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::SetReuseAddr
* 	Summary:	select whether to force use of the address
*	Args:		SOCKET socket
*					target socket
*				bool optVal
*					whether or not to use
----------------------------------------------------------------- */
bool SocketCore::SetReuseAddr(SOCKET socket, bool optVal)
{
	return SetSockOpt<int>(socket, SOL_SOCKET, SO_REUSEADDR, optVal);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::SetUpdateAcceptSocket
* 	Summary:	Allocate the properties of the listen socket 
*				to the target socket
*	Args:		SOCKET socket
*					target socket
*				SOCKET listenSocket
*					listen socket
----------------------------------------------------------------- */
bool SocketCore::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt<SOCKET>(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::CreateSocket
* 	Summary:	Create socket with WSA_FLAG_OVERLAPPED option
----------------------------------------------------------------- */
SOCKET SocketCore::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::BindWindowsFunction
* 	Summary:	bind windows function
----------------------------------------------------------------- */
bool SocketCore::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), &bytes, NULL, NULL);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::BindRioFunctionTable
* 	Summary:	bind rio function table
----------------------------------------------------------------- */
bool SocketCore::BindRioFunctionTable(SOCKET socket, GUID guid, RIO_EXTENSION_FUNCTION_TABLE& fnTable)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), (void**)&fnTable, sizeof(fnTable), &bytes, NULL, NULL);
}

/* -----------------------------------------------------------------
*	Method:		SocketCore::SetSockOpt
* 	Summary:	setsockopt wrapper method
----------------------------------------------------------------- */
template<typename T>
bool SocketCore::SetSockOpt(SOCKET socket, int level, int optName, T optVal)
{
	if (socket == INVALID_SOCKET)
	{
		return false;
	}

	return SOCKET_ERROR != setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}
