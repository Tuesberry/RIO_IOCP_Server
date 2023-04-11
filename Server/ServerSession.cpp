#pragma once
#include "pch.h"
#include "ServerSession.h"
#include "Utils/BufferHelper.h"
#include "ServerPacketHandler.h"
#include "Room.h"

ServerSession::ServerSession()
    : m_connectClientId(0)
    , m_moveTime(0)
    , m_loginTime(0)
    , m_serverProcessTime(0)
{
}

ServerSession::~ServerSession()
{
    //cout << "~ServerSession" << endl;
}

void ServerSession::OnRecvPacket(BYTE* buffer, int len)
{
    bool result = ServerPacketHandler::HandlePacket(
        static_pointer_cast<ServerSession>(shared_from_this()), buffer, len);
}

void ServerSession::OnSend(int len)
{
    //cout << "OnSend Len = " << len << endl;
}

void ServerSession::OnDisconnected()
{
    gRoom.Logout(m_connectClientId);
}

void ServerSession::SendMoveMsg(int targetId, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_MOVE));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_MOVE pktMove;
    pktMove.header.id = PROTO_ID::S2C_MOVE;
    pktMove.header.size = sizeof(PKT_S2C_MOVE);
    pktMove.id = m_connectClientId;
    pktMove.targetId = targetId;
    pktMove.x = x;
    pktMove.y = y;
    pktMove.moveTime = NULL;
    pktMove.processTime = NULL;

    if (targetId == m_connectClientId)
    {
        // 자기 자신의 move message인 경우
        pktMove.moveTime = m_moveTime;
        pktMove.processTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_serverProcessTime;
        gRoom.m_updateCnt++;
    }

    bw.Write(&pktMove, sizeof(PKT_S2C_MOVE));

    sendBuffer->OnWrite(sizeof(PKT_S2C_MOVE));
    Send(sendBuffer);
}

void ServerSession::SendEnterMsg(int targetId, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_ENTER));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_ENTER pktEnter;
    pktEnter.header.id = PROTO_ID::S2C_ENTER;
    pktEnter.header.size = sizeof(PKT_S2C_ENTER);
    pktEnter.id = m_connectClientId;
    pktEnter.targetId = targetId;
    pktEnter.x = x;
    pktEnter.y = y;

    bw.Write(&pktEnter, sizeof(PKT_S2C_ENTER));

    sendBuffer->OnWrite(sizeof(PKT_S2C_ENTER));
    Send(sendBuffer);
}

void ServerSession::SendLeaveMsg(int targetId)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_LEAVE));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_LEAVE pktLeave;
    pktLeave.header.id = PROTO_ID::S2C_LEAVE;
    pktLeave.header.size = sizeof(PKT_S2C_LEAVE);
    pktLeave.id = m_connectClientId;
    pktLeave.targetId = targetId;

    bw.Write(&pktLeave, sizeof(PKT_S2C_LEAVE));

    sendBuffer->OnWrite(sizeof(PKT_S2C_LEAVE));
    Send(sendBuffer);
}

void ServerSession::SendLoginResult(bool result, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_LOGIN_RESULT));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_LOGIN_RESULT pktResult;
    pktResult.header.id = PROTO_ID::LOGIN_RESULT;
    pktResult.header.size = sizeof(PKT_S2C_LOGIN_RESULT);
    pktResult.result = result;
    pktResult.id = m_connectClientId;
    pktResult.x = x;
    pktResult.y = y;
    pktResult.loginTime = m_loginTime;

    bw.Write(&pktResult, sizeof(PKT_S2C_LOGIN_RESULT));

    sendBuffer->OnWrite(sizeof(PKT_S2C_LOGIN_RESULT));
    Send(sendBuffer);
}
