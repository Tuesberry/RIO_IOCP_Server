#pragma once
#include "pch.h"
#include "ClientPacketHandler.h"
#include "ClientSession.h"
#include "Utils/BufferHelper.h"

bool ClientPacketHandler::HandlePacket(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case PROTO_ID::RESULT:
		return Handle_RESULT(session, buffer, len);
		break;
	default:
		break;
	}

	return false;
}

bool ClientPacketHandler::Handle_RESULT(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	if (header.size > len)
		return false;

	int id;
	br >> id;
	if (id != session->m_sessionID)
		return false;

	bool result;
	br >> result;

	return result;
}
