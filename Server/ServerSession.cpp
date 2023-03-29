#pragma once
#include "pch.h"
#include "ServerSession.h"
#include "Utils/BufferHelper.h"
#include "ServerPacketHandler.h"

ServerSession::ServerSession()
    : m_connectID(0)
    , m_posX(0)
    , m_posY(0)
{
}

ServerSession::~ServerSession()
{
    cout << "~ServerSession" << endl;
}

void ServerSession::OnRecvPacket(BYTE* buffer, int len)
{
    bool result = ServerPacketHandler::HandlePacket(
        static_pointer_cast<ServerSession>(shared_from_this()), buffer, len);
    SendResult((result && m_connectID != 0));
}

void ServerSession::OnSend(int len)
{
    //cout << "OnSend Len = " << len << endl;
}

void ServerSession::SendResult(bool isOk)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_RESULT));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());
    
    PKT_RESULT pktResult;
    pktResult.header.id = PROTO_ID::RESULT;
    pktResult.header.size = sizeof(PKT_RESULT);
    pktResult.id = m_connectID;
    pktResult.result = isOk;

    bw.Write(&pktResult, sizeof(PKT_RESULT));

    sendBuffer->OnWrite(sizeof(PKT_RESULT));
    Send(sendBuffer);
}
