#pragma once
#include "pch.h"
#include "CoreCommon.h"

class ClientSession;

class ClientPacketHandler
{
public:
	static bool HandlePacket(shared_ptr<ClientSession> session, BYTE* buffer, int len);
private:
	static bool Handle_RESULT(shared_ptr<ClientSession> session, BYTE* buffer, int len);
};