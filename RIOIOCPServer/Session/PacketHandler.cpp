#include "PacketHandler.h"
#include "../Contents/Room.h"
#include "RioServerSession.h"

shared_ptr<NetBuffer> BufferHandler::CreateNetBuffer(Protocol::S2C_LOGIN_RESULT& pkt)
{
	return _CreateNetBuffer(pkt, PROTO_ID::S2C_LOGIN_RESULT);
}

shared_ptr<NetBuffer> BufferHandler::CreateNetBuffer(Protocol::S2C_MOVE& pkt)
{
	return _CreateNetBuffer(pkt, PROTO_ID::S2C_MOVE);
}

shared_ptr<NetBuffer> BufferHandler::CreateNetBuffer(Protocol::S2C_ENTER& pkt)
{
	return _CreateNetBuffer(pkt, PROTO_ID::S2C_ENTER);
}

shared_ptr<NetBuffer> BufferHandler::CreateNetBuffer(Protocol::S2C_LEAVE& pkt)
{
	return _CreateNetBuffer(pkt, PROTO_ID::S2C_LEAVE);
}

shared_ptr<NetBuffer> BufferHandler::CreateNetBuffer(Protocol::S2C_MOVE_RESULT& pkt)
{
	return _CreateNetBuffer(pkt, PROTO_ID::S2C_MOVE_RESULT);
}

shared_ptr<NetBuffer> BufferHandler::CreateNetBuffer(Protocol::S2C_CHAT& pkt)
{
	return  _CreateNetBuffer(pkt, PROTO_ID::S2C_CHAT);
}
