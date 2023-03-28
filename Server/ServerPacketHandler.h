#pragma once
#include "pch.h"
#include "CoreCommon.h"
#include "Network/Packet.h"

class ServerSession;

class ServerPacketHandler
{
public:
	static bool HandlePacket(shared_ptr<ServerSession>session, BYTE* buffer, int len);
private:
	static bool Handle_LOGIN(shared_ptr<ServerSession>session, BYTE* buffer, int len);
	static bool Handle_INFO(shared_ptr<ServerSession>session, BYTE* buffer, int len);
};