#include "ClientPacketHandler.h"
#include "ClientSession.h"
#include "../StressTest/TestSessionManager.h"
#include "../StressTest/DelayManager.h"

/* --------------------------------------------------------
*	Method:		ClientPacketHandler::HandlePacket
*	Summary:	handle packet
*	Args:		shared_ptr<ClientSession> session
*					owner of packet
*				BYTE* buffer
*					byte that transfered from server
*				int len
*					buffer size
-------------------------------------------------------- */
bool ClientPacketHandler::HandlePacket(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));

	switch (header.id)
	{
	case PROTO_ID::S2C_MOVE_RESULT:
		return Handle_SC2_MOVE_RESULT(session, buffer, len);
		break;
	case PROTO_ID::S2C_MOVE:
		//return Handle_S2C_MOVE(session, buffer, len);
		return true;
		break;
	case PROTO_ID::S2C_ENTER:
		//return Handle_S2C_ENTER(session, buffer, len);
		return true;
		break;
	case PROTO_ID::S2C_LEAVE:
		//return Handle_S2C_LEAVE(session, buffer, len);
		return true;
		break;
	case PROTO_ID::S2C_LOGIN_RESULT:
		return Handle_LOGIN_RESULT(session, buffer, len);
		break;
	default:
		break;
	}

	return false;
}

/* --------------------------------------------------------
*	Method:		ClientPacketHandler::Handle_S2C_MOVE_RESULT
*	Summary:	handle S2C_MOVE_RESULT packet
*	Args:		shared_ptr<ClientSession> session
*					owner of packet
*				BYTE* buffer
*					byte that transfered from server
*				int len
*					buffer size
-------------------------------------------------------- */
bool ClientPacketHandler::Handle_SC2_MOVE_RESULT(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));

	// check packet size
	if (header.size > len)
	{
		return false;
	}

	Protocol::S2C_MOVE_RESULT pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	// check id validation
	if (pkt.session_id() != session->m_sessionID)
	{
		return false;
	}

	// update client-server pakcet transfer delay
	// - read delay data
	int moveStartTime = pkt.time_stamp();
	int processingTime = pkt.process_time();
	int sendTime = pkt.send_time();
	int recvStartTime = pkt.recv_time();

	int moveTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - moveStartTime;
	int recvTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - recvStartTime;

	// - update delay using delayManager
	gDelayMgr.m_avgProcessDelay.UpdateAvgDelay(processingTime);
	gDelayMgr.m_avgSendRecvDelay.UpdateAvgDelay(moveTime);
	gDelayMgr.m_avgSendingDelay.UpdateAvgDelay(sendTime);
	gDelayMgr.m_avgReceivingDelay.UpdateAvgDelay(recvTime);
	gDelayMgr.m_recvCnt.fetch_add(1);

	return true;
}

/* --------------------------------------------------------
*	Method:		ClientPacketHandler::Handle_S2C_MOVE
*	Summary:	handle S2C_MOVE packet
*	Args:		shared_ptr<ClientSession> session
*					owner of packet
*				BYTE* buffer
*					byte that transfered from server
*				int len
*					buffer size
-------------------------------------------------------- */
bool ClientPacketHandler::Handle_S2C_MOVE(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	return true;
}

/* --------------------------------------------------------
*	Method:		ClientPacketHandler::Handle_S2C_ENTER
*	Summary:	handle S2C_ENTER packet
*	Args:		shared_ptr<ClientSession> session
*					owner of packet
*				BYTE* buffer
*					byte that transfered from server
*				int len
*					buffer size
-------------------------------------------------------- */
bool ClientPacketHandler::Handle_S2C_ENTER(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	return true;
}

/* --------------------------------------------------------
*	Method:		ClientPacketHandler::Handle_S2C_LEAVE
*	Summary:	handle S2C_LEAVE packet
*	Args:		shared_ptr<ClientSession> session
*					owner of packet
*				BYTE* buffer
*					byte that transfered from server
*				int len
*					buffer size
-------------------------------------------------------- */
bool ClientPacketHandler::Handle_S2C_LEAVE(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	return true;
}

/* --------------------------------------------------------
*	Method:		ClientPacketHandler::Handle_LOGIN_RESULT
*	Summary:	handle LOGIN_RESULT packet
*	Args:		shared_ptr<ClientSession> session
*					owner of packet
*				BYTE* buffer
*					byte that transfered from server
*				int len
*					buffer size
-------------------------------------------------------- */
bool ClientPacketHandler::Handle_LOGIN_RESULT(shared_ptr<ClientSession> session, BYTE* buffer, int len)
{
	PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));

	// check packet size
	if (header.size > len)
	{
		return false;
	}

	Protocol::S2C_LOGIN_RESULT pkt;
	pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	// session id
	session->m_sessionID = pkt.session_id();

	// check result
	if (!pkt.result())
	{
		return false;
	}

	// login complete
	session->m_bLogin = true;

	// get player initial position
	Protocol::PLAYER_POS_INFO posInfo = pkt.pos_info();
	PositionInfo& pos = session->m_moveComp.m_positionInfo;
	pos.x = posInfo.x();
	pos.y = posInfo.y();
	pos.z = posInfo.z();
	pos.vx = posInfo.vx();
	pos.vy = posInfo.vy();
	pos.vz = posInfo.vz();
	pos.yaw = posInfo.yaw();
	pos.pitch = posInfo.pitch();
	pos.roll = posInfo.roll();
	pos.ax = posInfo.ax();
	pos.ay = posInfo.ay();
	pos.az = posInfo.az();

	// update login delay
	int loginTryTime = pkt.login_time();

	int loginDelay = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - loginTryTime;
	gDelayMgr.m_avgLoginDelay.UpdateAvgDelay(loginDelay);
	gDelayMgr.m_recvCnt.fetch_add(1);

	return true;
}

shared_ptr<SendBuffer> ClientPacketHandler::CreateSendBuffer(Protocol::C2S_LOGIN& pkt)
{
	return _CreateSendBuffer(pkt, PROTO_ID::C2S_LOGIN);
}

shared_ptr<SendBuffer> ClientPacketHandler::CreateSendBuffer(Protocol::C2S_MOVE& pkt)
{
	return _CreateSendBuffer(pkt, PROTO_ID::C2S_MOVE);
}
