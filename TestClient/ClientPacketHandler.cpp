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
	case PROTO_ID::S2C_MOVE:
		return Handle_S2C_MOVE(session, buffer, len);
		break;
	case PROTO_ID::S2C_ENTER:
		return Handle_S2C_ENTER(session, buffer, len);
		break;
	case PROTO_ID::S2C_LEAVE:
		return Handle_S2C_LEAVE(session, buffer, len);
		break;
	default:
		break;
	}

	return false;
}

bool ClientPacketHandler::Handle_S2C_MOVE(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	// check packet size
	if (header.size > len)
		return false;

	// check id validation
	int id;
	br >> id;
	if (id != session->m_sessionID)
		return true;

	br >> session->m_posX >> session->m_posY;

	cout << session->m_posX << " " << session->m_posY << endl;

	return true;
}

bool ClientPacketHandler::Handle_S2C_ENTER(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	// check packet size
	if (header.size > len)
		return false;

	// check id validation
	int id;
	br >> id;
	if (id == session->m_sessionID)
		return false;

	return true;
}

bool ClientPacketHandler::Handle_S2C_LEAVE(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	// check packet size
	if (header.size > len)
		return false;

	// check id validation
	int id;
	br >> id;
	if (id == session->m_sessionID)
		return false;

	return true;
}
