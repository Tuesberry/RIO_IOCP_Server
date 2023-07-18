#pragma once

#include "../ServerCommon.h"

#include "Network/PacketHeader.h"
#include "Network/SendBuffer.h"
#include "../Protocol/Protocol.pb.h"

class RioServerSession;

/* --------------------------------------------------------
*	Function:	_CreateSendBuffer
*	Summary:	create sendBuffer attaching PacketHeader
-------------------------------------------------------- */
template<typename T>
shared_ptr<SendBuffer> _CreateSendBuffer(T& pkt, unsigned short pktId)
{
	const unsigned short dataSize = static_cast<unsigned short>(pkt.ByteSizeLong());
	const unsigned short packetSize = dataSize + sizeof(PacketHeader);

	shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(packetSize);
	PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->GetData());
	header->id = pktId;
	header->size = packetSize;

	pkt.SerializeToArray(&header[1], dataSize);
	sendBuffer->OnWrite(packetSize);

	return sendBuffer;
}

/* --------------------------------------------------------
*	class:		ServerPacketHandler
*	Summary:	server packet handler
-------------------------------------------------------- */
class ServerPacketHandler
{
public:
	static bool HandlePacket(shared_ptr<RioServerSession>session, BYTE* buffer, int len);
private:
	static bool Handle_LOGIN(shared_ptr<RioServerSession>session, BYTE* buffer, int len);
	static bool Handle_C2S_MOVE(shared_ptr<RioServerSession>session, BYTE* buffer, int len);
	static bool Handle_LOGOUT(shared_ptr<RioServerSession>session, BYTE* buffer, int len);

public:
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::S2C_LOGIN_RESULT& pkt);
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::S2C_MOVE& pkt);
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::S2C_ENTER& pkt);
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::S2C_LEAVE& pkt);
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::S2C_MOVE_RESULT& pkt);
};

