#pragma once
#include "pch.h"
#include "ClientPacketHandler.h"
#include "ClientSession.h"
#include "Utils/BufferHelper.h"
#include "StressTestClient.h"

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

	unsigned int prevMoveTime = 0;
	br >> prevMoveTime;

	//cout << session->m_posX << " " << session->m_posY << endl;
	if (id == targetId)
	{
		unsigned int prevTime = session->m_moveTime;
		session->m_moveTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - prevMoveTime;
		
		// check delay > delay_limit
		gDelayMgr.UpdateDelay(static_cast<int>(session->m_moveTime));
		// cout << id << " " << targetId << " " << static_cast<int>(session->m_moveTime) << endl;
		// update avg delay
		if (session->m_bAddDelay == false)
		{
			gDelayMgr.AddNewInAvgDelay(session->m_moveTime);
			session->m_bAddDelay = true;
		}
		else
		{
			gDelayMgr.UpdateAvgDelay(session->m_moveTime, prevTime);
		}

		gDelayMgr.m_updateCnt++;
		session->SendMove();
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

	br >> session->m_posX >> session->m_posY;

	//cout << session->m_posX << " " << session->m_posY << endl;
	//cout << "login" << endl;

	session->SendMove();

	return true;
}
