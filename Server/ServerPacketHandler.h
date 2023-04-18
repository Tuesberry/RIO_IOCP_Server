#pragma once

#include "Common.h"

#include "Network/Packet.h"

#if IOCP
class IocpServerSession;
#else RIO
class RioServerSession;
#endif 

/* --------------------------------------------------------
*	class:		ServerPacketHandler
*	Summary:	server packet handler
-------------------------------------------------------- */

class ServerPacketHandler
{
#if IOCP
	// IOCP Packet Handler
public:
	static bool HandlePacket(shared_ptr<IocpServerSession>session, BYTE* buffer, int len);
private:
	static bool Handle_LOGIN(shared_ptr<IocpServerSession>session, BYTE* buffer, int len);
	static bool Handle_C2S_MOVE(shared_ptr<IocpServerSession>session, BYTE* buffer, int len);
	static bool Handle_LOGOUT(shared_ptr<IocpServerSession>session, BYTE* buffer, int len);

#else RIO
	// RIO Packet Handler
public:
	static bool HandlePacket(shared_ptr<RioServerSession>session, BYTE* buffer, int len);
private:
	static bool Handle_LOGIN(shared_ptr<RioServerSession>session, BYTE* buffer, int len);
	static bool Handle_C2S_MOVE(shared_ptr<RioServerSession>session, BYTE* buffer, int len);
	static bool Handle_LOGOUT(shared_ptr<RioServerSession>session, BYTE* buffer, int len);
#endif

};
