#include "ServerPacketHandler.h"

#include "Utils/BufferHelper.h"
#include "Network/IocpServer.h"
#include "Room.h"

#if IOCP
#include "IocpServerSession.h"
#else RIO
#include "RioServerSession.h"
#endif 

#if IOCP

bool ServerPacketHandler::HandlePacket(shared_ptr<IocpServerSession>session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case PROTO_ID::LOGIN:
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

bool ServerPacketHandler::Handle_LOGIN(shared_ptr<IocpServerSession>session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	if (header.size > len)
		return false;

	br >> session->m_connectClientId >> session->m_loginTime;

	// create new player
	shared_ptr<Player> player = make_shared<Player>(session->m_connectClientId, session);
	session->m_ownPlayer = player;
	// login
	gRoom->DoAsync(&Room::Login, player);

	return true;
}

bool ServerPacketHandler::Handle_C2S_MOVE(shared_ptr<IocpServerSession>session, BYTE* buffer, int len)
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
	gRoom->DoAsync(&Room::MovePlayer, session->m_ownPlayer, direction);

	return true;
}

bool ServerPacketHandler::Handle_LOGOUT(shared_ptr<IocpServerSession> session, BYTE* buffer, int len)
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
#else RIO
bool ServerPacketHandler::HandlePacket(shared_ptr<RioServerSession>session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case PROTO_ID::LOGIN:
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

bool ServerPacketHandler::Handle_LOGIN(shared_ptr<RioServerSession>session, BYTE* buffer, int len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	if (header.size > len)
		return false;

	br >> session->m_connectClientId >> session->m_loginTime;

	// create new player
	shared_ptr<Player> player = make_shared<Player>(session->m_connectClientId, session);
	session->m_ownPlayer = player;
	// login
	gRoom->DoAsync(&Room::Login, player);

	return true;
}

bool ServerPacketHandler::Handle_C2S_MOVE(shared_ptr<RioServerSession>session, BYTE* buffer, int len)
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
	gRoom->DoAsync(&Room::MovePlayer, session->m_ownPlayer, direction);

	return true;
}

bool ServerPacketHandler::Handle_LOGOUT(shared_ptr<RioServerSession> session, BYTE* buffer, int len)
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
#endif
