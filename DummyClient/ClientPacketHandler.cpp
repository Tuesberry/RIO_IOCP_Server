#include "ClientPacketHandler.h"
#include "ClientSession.h"
#include "Utils/BufferHelper.h"
#include "StressTestClient.h"
#include "DelayManager.h"

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
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case PROTO_ID::S2C_MOVE:
		return Handle_S2C_MOVE(session, buffer, len);
		break;
	case PROTO_ID::S2C_ENTER:
		//return Handle_S2C_ENTER(session, buffer, len);
		return true;
		break;
	case PROTO_ID::S2C_LEAVE:
		//return Handle_S2C_LEAVE(session, buffer, len);
		return true;
		break;
	case PROTO_ID::LOGIN_RESULT:
		return Handle_LOGIN_RESULT(session, buffer, len);
		break;
	default:
		break;
	}

	return false;
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
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	// check packet size
	if (header.size > len)
		return false;

	// check id validation
	int id;
	int targetId;
	br >> id >> targetId;
	if (id != session->m_sessionID)
		return false;

	// In this time, other clients movement packets are not considered
	// for stress test!
	if (id != targetId)
		return true;

	// read position
	br >> session->m_posX >> session->m_posY;

	// update client-server pakcet transfer delay
	// - read delay data
	int moveStartTime = 0;
	int processingTime = 0;

	br >> moveStartTime >> processingTime;

	int moveTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - moveStartTime;

	// - update delay using delayManager
	gDelayMgr.m_avgProcessDelay.UpdateAvgDelay(processingTime);
	gDelayMgr.m_avgSendingDelay.UpdateAvgDelay(moveTime);
	gDelayMgr.m_recvCnt.fetch_add(1);

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
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	// check packet size
	if (header.size > len)
		return false;

	// check id validation
	int id;
	int targetId;
	br >> id >> targetId;
	if (id != session->m_sessionID)
		return false;

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
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	// check packet size
	if (header.size > len)
		return false;

	// check id validation
	int id;
	int targetId;
	br >> id >> targetId;
	if (id != session->m_sessionID)
		return false;

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
		return false;

	// check result
	bool result;
	br >> result;
	if (result == false)
		return false;

	// login complete
	session->m_bLogin = true;

	// get player initial position
	br >> session->m_posX >> session->m_posY;

	// update login delay
	int loginTryTime;
	br >> loginTryTime;

	int loginDelay = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - loginTryTime;
	gDelayMgr.m_avgLoginDelay.UpdateAvgDelay(loginDelay);
	gDelayMgr.m_recvCnt.fetch_add(1);

	return true;
}
