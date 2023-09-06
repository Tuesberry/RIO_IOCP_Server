#pragma once

#include "../DummyClientCommon.h"

#include "Network/PacketHeader.h"
#include "Network/NetBuffer.h"
#include "../Protocol/ProtoId.h"
#include "../Protocol/Protocol.pb.h"

class ClientSession;

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
*	class:		PacketHandler
*	Summary:	packet handler for client packet
-------------------------------------------------------- */
class PacketHandler
{
public:
	static bool HandlePacket(shared_ptr<ClientSession> session, char* buffer, int len);
private:
	static bool Handle_SC2_MOVE_RESULT(shared_ptr<ClientSession> session, char* buffer, int len);
	static bool Handle_S2C_MOVE(shared_ptr<ClientSession> session, char* buffer, int len);
	static bool Handle_S2C_ENTER(shared_ptr<ClientSession> session, char* buffer, int len);
	static bool Handle_S2C_LEAVE(shared_ptr<ClientSession> session, char* buffer, int len);
	static bool Handle_LOGIN_RESULT(shared_ptr<ClientSession> session, char* buffer, int len);

public:
	static shared_ptr<NetBuffer> CreateNetBuffer(Protocol::C2S_LOGIN& pkt);
	static shared_ptr<NetBuffer> CreateNetBuffer(Protocol::C2S_MOVE& pkt);
};