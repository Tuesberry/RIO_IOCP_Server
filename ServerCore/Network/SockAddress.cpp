#include "SockAddress.h"

SockAddress::SockAddress(const char* ip, unsigned short port)
{
	memset(&m_sockAddress, 0, sizeof(m_sockAddress));
	m_sockAddress.sin_family = AF_INET;
	::inet_pton(AF_INET, ip, &m_sockAddress.sin_addr);
	m_sockAddress.sin_port = htons(port);
}

SockAddress::SockAddress(SOCKADDR_IN sockAddr)
	:m_sockAddress(sockAddr)
{
}

SOCKADDR_IN& SockAddress::GetSockAddr()
{
	return m_sockAddress;
}

int SockAddress::GetAddrSize()
{
	return sizeof(m_sockAddress);
}

wstring SockAddress::GetIpAddress()
{
	WCHAR ipaddr[INET_ADDRSTRLEN];
	::InetNtopW(AF_INET, &m_sockAddress.sin_addr, ipaddr, sizeof(ipaddr) / sizeof(WCHAR));
	return wstring(ipaddr);
}

void SockAddress::SetIpAddress(const char* ip)
{
	::inet_pton(AF_INET, ip, &m_sockAddress.sin_addr);
}

unsigned short SockAddress::GetPortNum()
{
	return ntohs(m_sockAddress.sin_port);
}

void SockAddress::SetPortNum(unsigned short port)
{
	m_sockAddress.sin_port = htons(port);
}
