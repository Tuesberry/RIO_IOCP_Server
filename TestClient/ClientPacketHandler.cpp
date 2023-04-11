#pragma once
#include "pch.h"
#include "ClientPacketHandler.h"
#include "ClientSession.h"
#include "Utils/BufferHelper.h"
#include "StressTestClient.h"
#include "DelayManager.h"
#include "DelayWriteManager.h"

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
	case PROTO_ID::LOGIN_RESULT:
		return Handle_LOGIN_RESULT(session, buffer, len);
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
	int targetId;
	br >> id >> targetId;
	if (id != session->m_sessionID)
		return false;

	br >> session->m_posX >> session->m_posY;

	// update client-server pakcet transfer delay
	if (id == targetId)
	{
		int prevMoveTime = 0;
		br >> prevMoveTime;

		int prevProcessTime = session->m_processTime;
		br >> session->m_processTime;
		//cout << session->m_processTime << endl;

		unsigned int prevTime = session->m_moveTime;
		session->m_moveTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - prevMoveTime;
		//cout << session->m_moveTime << endl;
		if (session->m_bAddDelay == false)
		{
			gDelayMgr.AddNewInAvgDelay(session->m_moveTime);
			gDelayMgr.AddNewInAvgProcessTime(session->m_processTime);
			session->m_bAddDelay = true;
		}
		else
		{
			gDelayMgr.UpdateAvgDelay(session->m_moveTime, prevTime);
			gDelayMgr.UpdateAvgProcessTime(session->m_processTime, prevProcessTime);
		}
		gDelayMgr.m_updateCnt++;

		// write file
		string data = to_string(session->m_moveTime) + " " + to_string(session->m_processTime);
		gDelayWriteMgr.addData(data);
	}

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
	int targetId;
	br >> id >> targetId;
	if (id != session->m_sessionID)
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
	int targetId;
	br >> id >> targetId;
	if (id != session->m_sessionID)
		return false;

	return true;
}

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

	bool result;
	br >> result;
	if (result == false)
		return false;

	// login complete
	session->m_bLogin = true;

	// get player initial position
	br >> session->m_posX >> session->m_posY;

	// update login delay
	unsigned int loginTryTime;
	br >> loginTryTime;

	unsigned int loginDelay = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - loginTryTime;
	gDelayMgr.UpdateLoginDelay(loginDelay);

	return true;
}
