#pragma once
#include "Common.h"
#include "SockAddress.h"

/* ----------------------------
*	SocketCore
---------------------------- */

class SocketCore
{
public:
	SocketCore();
	SocketCore(const SocketCore& other) = delete;
	SocketCore(SocketCore&& other) = delete;
	SocketCore& operator=(const SocketCore& other) = delete;
	SocketCore& operator=(SocketCore&& other) = delete;
	~SocketCore() = default;

	static bool Init();
	static bool Clear();
	static SOCKET Socket();
	static bool Close(SOCKET& socket);

	// server
	static bool Bind(SOCKET socket, SockAddress sockAddr);
	static bool BindAddrAny(SOCKET socket, unsigned short port);
	static bool Listen(SOCKET socket);
	static SOCKET Accept(SOCKET socket);

	// client
	static bool Connect(SOCKET socket, SockAddress& sockAddr);

	// send & recv
	static int Send(SOCKET socket, char* buf, int size);
	static int Recv(SOCKET socket, char* buf, int size);

	// set socket option
	static bool SetNodelay(SOCKET socket, bool optVal);
	static bool SetLinger(SOCKET socket, bool optVal, int time);
	static bool SetSendBufSiz(SOCKET socket, int size);
	static bool SetRecvBufSiz(SOCKET socket, int size);
	static bool SetReuseAddr(SOCKET socket, bool optVal);
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

private:
	static SOCKET CreateSocket();
	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);

	template<typename T>
	inline static bool SetSockOpt(SOCKET socket, int level, int optName, T optVal);

public:
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_DISCONNECTEX DisconnectEx;
	static LPFN_ACCEPTEX AcceptEx;

private:
	SOCKET m_socket;
};