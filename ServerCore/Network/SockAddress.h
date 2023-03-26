#pragma once
#include "Common.h"

/* ----------------------------
*		SockAddress
---------------------------- */

class SockAddress
{
public:
	SockAddress() = default;

	SockAddress(wstring ip, unsigned short port);
	SockAddress(SOCKADDR_IN sockAddr);

	~SockAddress() = default;

	SOCKADDR_IN& GetSockAddr();
	int GetAddrSize();

	wstring GetIpAddress();
	void SetIpAddress(wstring ip);

	unsigned short GetPortNum();
	void SetPortNum(unsigned short port);

private:
	SOCKADDR_IN m_sockAddress = {};
};