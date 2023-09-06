#include "SockAddress.h"

/* -----------------------------------------------------------------
*	Method:		SockAddress::SockAddress
*	Summary:	Constructor
*	Args:		string ip
*					ip address of socket
*				unsigned short port
*					port num of socket
----------------------------------------------------------------- */
SockAddress::SockAddress(string ip, unsigned short port)
{
	memset(&m_sockAddress, 0, sizeof(m_sockAddress));
	m_sockAddress.sin_family = AF_INET;
	::inet_pton(AF_INET, ip.c_str(), &m_sockAddress.sin_addr);
	m_sockAddress.sin_port = htons(port);
}

/* -----------------------------------------------------------------
*	Method:		SockAddress::SockAddress
*	Summary:	Constructor
*	Args:		SOCKADDR_IN sockAddr
*					sockAddr to use
----------------------------------------------------------------- */
SockAddress::SockAddress(SOCKADDR_IN sockAddr)
	:m_sockAddress(sockAddr)
{
}

/* -----------------------------------------------------------------
*	Method:		SockAddress::GetSockAddr
*	Summary:	return reference of the SOCKADDR_IN variable
----------------------------------------------------------------- */
SOCKADDR_IN& SockAddress::GetSockAddr()
{
	return m_sockAddress;
}

/* -----------------------------------------------------------------
*	Method:		SockAddress::GetAddrSize
*	Summary:	return the size of the m_sockAddress
----------------------------------------------------------------- */
int SockAddress::GetAddrSize()
{
	return sizeof(m_sockAddress);
}

/* -----------------------------------------------------------------
*	Method:		SockAddress::GetIpAddress
*	Summary:	return ip address of the socket address
----------------------------------------------------------------- */
string SockAddress::GetIpAddress()
{
	char ipaddr[INET_ADDRSTRLEN];
	::inet_ntop(AF_INET, &m_sockAddress.sin_addr, ipaddr, sizeof(ipaddr) / sizeof(WCHAR));
	return string(ipaddr);
}

/* -----------------------------------------------------------------
*	Method:		SockAddress::SetIpAddress
*	Summary:	set ip address of the socket address
*	Args:		string ip
*					ip address to change
----------------------------------------------------------------- */
void SockAddress::SetIpAddress(string ip)
{
	::inet_pton(AF_INET, ip.c_str(), &m_sockAddress.sin_addr);
}

/* -----------------------------------------------------------------
*	Method:		SockAddress::GetPortNum
*	Summary:	return port number of the socket address
----------------------------------------------------------------- */
unsigned short SockAddress::GetPortNum()
{
	return ntohs(m_sockAddress.sin_port);
}

/* -----------------------------------------------------------------
*	Method:		SockAddress::SetPortNum
*	Summary:	set port number of the socket address
*	Args:		unsigned short port
*					port number to change
----------------------------------------------------------------- */
void SockAddress::SetPortNum(unsigned short port)
{
	m_sockAddress.sin_port = htons(port);
}
