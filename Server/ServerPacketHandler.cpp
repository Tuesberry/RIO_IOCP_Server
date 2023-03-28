#pragma once
#include "pch.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"
#include "Utils/BufferHelper.h"

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
	case PROTO_ID::INFO :
		return Handle_INFO(session, buffer, len);
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

	br >> session->m_connectID;
	
	cout << session->m_connectID << " : login" << endl;

	return true;
}

bool ServerPacketHandler::Handle_INFO(shared_ptr<ServerSession>session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	if (header.size > len)
		return false;

	int id;
	br >> id;
	if (id != session->m_connectID)
		return false;

	br >> session->m_posX >> session->m_posY;

	cout << session->m_connectID << " : send Info" << endl;
	cout << session->m_posX << " " << session->m_posY << endl;

	return true;
}
