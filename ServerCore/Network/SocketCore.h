#pragma once
#include "pch.h"
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

	bool Init();
	bool Clear();
	bool Socket();
	bool Close();

	// server
	bool Bind(SockAddress& sockAddr);
	bool Listen();
	SOCKET Accept();

	// client
	bool Connect(SockAddress& sockAddr);

	// send & recv
	int Send(char* buf, int size);
	int Recv(char* buf, int size);
	
	// set socket option
	void SetNodelay(bool optVal);
	void SetLinger(bool optVal, int time);
	void SetSendBufSiz(int size);
	void SetRecvBufSiz(int size);

private:
	SOCKET CreateSocket();
	bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);

	template<typename T>
	void SetSockOpt(int level, int optName, T optVal);

public:
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_DISCONNECTEX DisconnectEx;
	static LPFN_ACCEPTEX AcceptEx;

private:
	SOCKET m_socket;
};