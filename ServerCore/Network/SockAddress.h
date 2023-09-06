#pragma once

#include "Common.h"

/* --------------------------------------------------------
*	class:		SockAddress
*	Summary:	Socket Address Wrapper Class
-------------------------------------------------------- */
class SockAddress
{
public:
	SockAddress() = default;

	SockAddress(string ip, unsigned short port);
	SockAddress(SOCKADDR_IN sockAddr);

	~SockAddress() = default;

	SOCKADDR_IN& GetSockAddr();
	int GetAddrSize();
	string GetIpAddress();
	unsigned short GetPortNum();

	void SetIpAddress(string ip);
	void SetPortNum(unsigned short port);

private:
	SOCKADDR_IN m_sockAddress = {};
};