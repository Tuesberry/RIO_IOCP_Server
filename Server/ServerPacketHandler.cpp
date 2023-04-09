#pragma once
#include "pch.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"
#include "Utils/BufferHelper.h"
#include "Network/IocpServer.h"
#include "Room.h"

bool ServerPacketHandler::HandlePacket(shared_ptr<ServerSession>session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case PROTO_ID::LOGIN :
		return Handle_LOGIN(session, buffer, len);
		break;
	case PROTO_ID::C2S_MOVE:
		return Handle_C2S_MOVE(session, buffer, len);
		break;
	case PROTO_ID::LOGOUT:
		return Handle_LOGOUT(session, buffer, len);
		break;
	default:
		break;
	}

	return false;
}

bool ServerPacketHandler::Handle_LOGIN(shared_ptr<ServerSession>session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	if (header.size > len)
		return false;

	br >> session->m_connectClientId >> session->m_loginTime;
	
	// login
	shared_ptr<Player> player = make_shared<Player>(session->m_connectClientId, session);
	gRoom.Login(player);

	return true;
}

bool ServerPacketHandler::Handle_C2S_MOVE(shared_ptr<ServerSession>session, BYTE* buffer, int len)
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
	if (id != session->m_connectClientId)
		return false;

	// get information
	unsigned short direction;
	br >> direction >> session->m_moveTime;

	// move player
	session->m_serverProcessTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
	gRoom.MovePlayer(session->m_connectClientId, direction);

	return true;
}

bool ServerPacketHandler::Handle_LOGOUT(shared_ptr<ServerSession> session, BYTE* buffer, int len)
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
	if (id != session->m_connectClientId)
		return false;

	// logout
	session->Disconnect();

	return true;
}
