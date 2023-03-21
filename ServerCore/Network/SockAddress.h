#pragma once
#include "Common.h"

/* ----------------------------
*		SockAddress
---------------------------- */

class SockAddress
{
public:
	SockAddress() = default;

	SockAddress(const char* ip, unsigned short port);
	SockAddress(SOCKADDR_IN sockAddr);

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