#include "SockAddress.h"

SockAddress::SockAddress(wstring ip, unsigned short port)
{
	memset(&m_sockAddress, 0, sizeof(m_sockAddress));
	m_sockAddress.sin_family = AF_INET;
	::InetPtonW(AF_INET, ip.c_str(), &m_sockAddress.sin_addr);
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

void SockAddress::SetIpAddress(wstring ip)
{
	::InetPtonW(AF_INET, ip.c_str(), &m_sockAddress.sin_addr);
}

unsigned short SockAddress::GetPortNum()
{
	return ntohs(m_sockAddress.sin_port);
}

void SockAddress::SetPortNum(unsigned short port)
{
	m_sockAddress.sin_port = htons(port);
}
