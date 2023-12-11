#pragma once

#include "../ServerCommon.h"

#include "../Contents/Room.h"
#include "../Contents/Player.h"
#include "Network/NetBuffer.h"
#include "Network/PacketHeader.h"
#include "../Protocol/ProtoId.h"
#include "../Protocol/Protocol.pb.h"

/* --------------------------------------------------------
*	Function:	_CreateNetBuffer
*	Summary:	create NetBuffer attaching PacketHeader
-------------------------------------------------------- */
template<typename T>
shared_ptr<NetBuffer> _CreateNetBuffer(T& pkt, unsigned short pktId)
{
	int dataSize = static_cast<int>(pkt.ByteSizeLong());
	int packetSize = dataSize + sizeof(PacketHeader);

	shared_ptr<NetBuffer> netBuffer = make_shared<NetBuffer>(packetSize);
	PacketHeader* header = reinterpret_cast<PacketHeader*>(netBuffer->GetBuffer());
	header->id = pktId;
	header->size = packetSize;

	pkt.SerializeToArray(&header[1], dataSize);
	netBuffer->OnWriteBuffer(packetSize);

	return netBuffer;
}

/* --------------------------------------------------------
*	class:		BufferHandler
*	Summary:	server buffer handler
-------------------------------------------------------- */
class BufferHandler
{
public:
	static shared_ptr<NetBuffer> CreateNetBuffer(Protocol::S2C_LOGIN_RESULT& pkt);
	static shared_ptr<NetBuffer> CreateNetBuffer(Protocol::S2C_MOVE& pkt);
	static shared_ptr<NetBuffer> CreateNetBuffer(Protocol::S2C_ENTER& pkt);
	static shared_ptr<NetBuffer> CreateNetBuffer(Protocol::S2C_LEAVE& pkt);
	static shared_ptr<NetBuffer> CreateNetBuffer(Protocol::S2C_MOVE_RESULT& pkt);
	static shared_ptr<NetBuffer> CreateNetBuffer(Protocol::S2C_CHAT& pkt);
};

/* --------------------------------------------------------
*	class:		PacketHandler
*	Summary:	server packet handler
-------------------------------------------------------- */
template<typename ServerSession>
class PacketHandler
{
public:
	static bool HandlePacket(shared_ptr<ServerSession>session, char* buffer, int len);
private:
	static bool Handle_LOGIN(shared_ptr<ServerSession>session, char* buffer, int len);
	static bool Handle_C2S_MOVE(shared_ptr<ServerSession>session, char* buffer, int len);
	static bool Handle_LOGOUT(shared_ptr<ServerSession>session, char* buffer, int len);
	static bool Handle_CHAT(shared_ptr<ServerSession>session, char* buffer, int len);
};

template<typename ServerSession>
bool PacketHandler<ServerSession>::HandlePacket(shared_ptr<ServerSession>session, char* buffer, int len)
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

/* --------------------------------------------------------
*	Method:		PacketHandler<ServerSession>::Handle_LOGIN
*	Summary:	handle LOGIN packet
-------------------------------------------------------- */
template<typename ServerSession>
bool PacketHandler<ServerSession>::Handle_LOGIN(shared_ptr<ServerSession>session, char* buffer, int len)
{
	PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));

	// check packet size
	if (header.size > len)
	{
		return false;
	}

	// packet 추출
	Protocol::C2S_LOGIN pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	session->m_playerId = pkt.player_id();
	session->m_sessionId = gRoom->m_sessionCnt.fetch_add(1);
	session->m_sessionId++;
	session->m_loginTime = pkt.login_time();

	// 로그
	// cout << "Recv Login | " << session->m_sessionId << endl;

	// create new player
	shared_ptr<Player> player = make_shared<Player>(session->m_sessionId, session);
	session->m_ownPlayer = player;

	player->m_playerInfo.playerType = pkt.player_type();
	player->m_playerInfo.playerId = pkt.player_id();

	// login
	//gRoom->DoAsync(&Room::Login, player);
	gRoom->Login(player);

	return true;
}

/* --------------------------------------------------------
*	Method:		PacketHandler<ServerSession>::Handle_C2S_MOVE
*	Summary:	handle C2S_MOVE packet
-------------------------------------------------------- */
template<typename ServerSession>
bool PacketHandler<ServerSession>::Handle_C2S_MOVE(shared_ptr<ServerSession>session, char* buffer, int len)
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

/* --------------------------------------------------------
*	Method:		PacketHandler<ServerSession>::Handle_LOGOUT
*	Summary:	handle LOGOUT packet
-------------------------------------------------------- */
template<typename ServerSession>
bool PacketHandler<ServerSession>::Handle_LOGOUT(shared_ptr<ServerSession> session, char* buffer, int len)
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

/* --------------------------------------------------------
*	Method:		PacketHandler<ServerSession>::Handle_CHAT
*	Summary:	handle CHAT packet
-------------------------------------------------------- */
template<typename ServerSession>
bool PacketHandler<ServerSession>::Handle_CHAT(shared_ptr<ServerSession> session, char* buffer, int len)
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