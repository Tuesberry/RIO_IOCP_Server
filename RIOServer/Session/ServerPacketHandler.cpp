#include "ServerPacketHandler.h"
#include "../Contents/Room.h"
#include "RioServerSession.h"

bool ServerPacketHandler::HandlePacket(shared_ptr<RioServerSession>session, BYTE* buffer, int len)
{
	PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));

	switch (header.id)
	{
	case PROTO_ID::C2S_LOGIN:
		return Handle_LOGIN(session, buffer, len);
		break;
	case PROTO_ID::C2S_LOGOUT:
		return Handle_LOGOUT(session, buffer, len);
		break;
	case PROTO_ID::C2S_MOVE:
		return Handle_C2S_MOVE(session, buffer, len);
		break;
	case PROTO_ID::C2S_CHAT:
		return Handle_CHAT(session, buffer, len);
		break;
	default:
		break;
	}

	return false;
}

bool ServerPacketHandler::Handle_LOGIN(shared_ptr<RioServerSession>session, BYTE* buffer, int len)
{
	PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));

	// check packet size
	if (header.size > len)
	{
		return false;
	}

	Protocol::C2S_LOGIN pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	session->m_playerId = pkt.player_id();
	session->m_sessionId = gRoom->m_sessionCnt.fetch_add(1);
	session->m_sessionId++;
	session->m_loginTime = pkt.login_time();

	cout << "Recv Login | " << session->m_sessionId << endl;

	// create new player
	shared_ptr<Player> player = make_shared<Player>(session->m_sessionId, session);
	session->m_ownPlayer = player;

	player->m_playerInfo.playerType = pkt.player_type();
	player->m_playerInfo.playerId = pkt.player_id();

	// login
	gRoom->DoAsync(&Room::Login, player);
	
	return true;
}

bool ServerPacketHandler::Handle_C2S_MOVE(shared_ptr<RioServerSession>session, BYTE* buffer, int len)
{
	PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));

	// check packet size
	if (header.size > len)
	{
		return false;
	}

	Protocol::C2S_MOVE pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	// check id validation
	if (pkt.session_id() != session->m_sessionId)
	{
		return false;
	}
	
	// new position info
	Protocol::PLAYER_POS_INFO newPosInfo = pkt.pos_info();
	PlayerInfo& pInfo = session->m_ownPlayer->m_playerInfo;

	pInfo.x = newPosInfo.x();
	pInfo.y = newPosInfo.y();
	pInfo.z = newPosInfo.z();
	pInfo.yaw = newPosInfo.yaw();
	pInfo.pitch = newPosInfo.pitch();
	pInfo.roll = newPosInfo.roll();
	pInfo.vx = newPosInfo.vx();
	pInfo.vy = newPosInfo.vy();
	pInfo.vz = newPosInfo.vz();
	pInfo.ax = newPosInfo.ax();
	pInfo.ay = newPosInfo.ay();
	pInfo.az = newPosInfo.az();
	pInfo.timeStamp = pkt.time_stamp();

	// request info?
	session->m_bRequestResult = pkt.request_result();

	// move time
	session->m_moveTime = pkt.time_stamp();
	// send time 
	session->m_sendTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - session->m_moveTime;

	// move player
	gRoom->MovePlayer(session->m_ownPlayer);

	return true;
}

bool ServerPacketHandler::Handle_LOGOUT(shared_ptr<RioServerSession> session, BYTE* buffer, int len)
{
	PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));

	// check packet size
	if (header.size > len)
	{
		return false;
	}

	Protocol::C2S_LOGOUT pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	// logout
	gRoom->DoAsync(&Room::Logout, session->m_ownPlayer);

	// disconnect session
	session->Disconnect();

	return true;
}

bool ServerPacketHandler::Handle_CHAT(shared_ptr<RioServerSession> session, BYTE* buffer, int len)
{
	PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));

	// check packet size
	if (header.size > len)
	{
		return false;
	}

	Protocol::C2S_CHAT pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	// check id validation
	if (pkt.session_id() != session->m_sessionId)
	{
		return false;
	}

	cout << "Recv Chat | " << session->m_sessionId << " : " << pkt.chat() << endl;

	// chat
	gRoom->Chat(session->m_ownPlayer, pkt.chat());

	return true;
}

shared_ptr<SendBuffer> ServerPacketHandler::CreateSendBuffer(Protocol::S2C_LOGIN_RESULT& pkt)
{
	return _CreateSendBuffer(pkt, PROTO_ID::S2C_LOGIN_RESULT);
}
shared_ptr<SendBuffer> ServerPacketHandler::CreateSendBuffer(Protocol::S2C_MOVE& pkt)
{
	return _CreateSendBuffer(pkt, PROTO_ID::S2C_MOVE);
}
shared_ptr<SendBuffer> ServerPacketHandler::CreateSendBuffer(Protocol::S2C_ENTER& pkt)
{
	return _CreateSendBuffer(pkt, PROTO_ID::S2C_ENTER);
}
shared_ptr<SendBuffer> ServerPacketHandler::CreateSendBuffer(Protocol::S2C_LEAVE& pkt)
{
	return _CreateSendBuffer(pkt, PROTO_ID::S2C_LEAVE);
}

shared_ptr<SendBuffer> ServerPacketHandler::CreateSendBuffer(Protocol::S2C_MOVE_RESULT& pkt)
{
	return _CreateSendBuffer(pkt, PROTO_ID::S2C_MOVE_RESULT);
}

shared_ptr<SendBuffer> ServerPacketHandler::CreateSendBuffer(Protocol::S2C_CHAT& pkt)
{
	return  _CreateSendBuffer(pkt, PROTO_ID::S2C_CHAT);
}

