#pragma once

#include "../DummyClientCommon.h"

#include "Network/PacketHeader.h"
#include "Network/SendBuffer.h"
#include "../Protocol/Protocol.pb.h"

class ClientSession;

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
*	class:		ClientPacketHandler
*	Summary:	packet handler for client packet
-------------------------------------------------------- */
class ClientPacketHandler
{
public:
	static bool HandlePacket(shared_ptr<ClientSession> session, BYTE* buffer, int len);
private:
	static bool Handle_SC2_MOVE_RESULT(shared_ptr<ClientSession> session, BYTE* buffer, int len);
	static bool Handle_S2C_MOVE(shared_ptr<ClientSession> session, BYTE* buffer, int len);
	static bool Handle_S2C_ENTER(shared_ptr<ClientSession> session, BYTE* buffer, int len);
	static bool Handle_S2C_LEAVE(shared_ptr<ClientSession> session, BYTE* buffer, int len);
	static bool Handle_LOGIN_RESULT(shared_ptr<ClientSession> session, BYTE* buffer, int len);

public:
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::C2S_LOGIN& pkt);
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::C2S_MOVE& pkt);
};