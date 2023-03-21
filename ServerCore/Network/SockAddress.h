#pragma once
#include "pch.h"

/* ----------------------------
*		SockAddress
---------------------------- */

class SockAddress
{
public:
	SockAddress() = delete;

	SockAddress(const char* ip, unsigned short port);
	SockAddress(SOCKADDR_IN sockAddr);

	SockAddress(const SockAddress& other) = delete;
	SockAddress(SockAddress&& other) = delete;
	SockAddress& operator=(const SockAddress& other) = delete;
	SockAddress& operator=(SockAddress&& other) = delete;
	~SockAddress() = default;

	SOCKADDR_IN& GetSockAddr();
	int GetAddrSize();

	wstring GetIpAddress();
	void SetIpAddress(const char* ip);

	unsigned short GetPortNum();
	void SetPortNum(unsigned short port);

private:
	SOCKADDR_IN m_sockAddress = {};
};