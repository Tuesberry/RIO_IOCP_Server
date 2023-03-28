#pragma once
#include "pch.h"
#include "ServerSession.h"
#include "Utils/BufferHelper.h"

ServerSession::~ServerSession()
{
    cout << "~ServerSession" << endl;
}

void ServerSession::OnRecvPacket(BYTE* buffer, int len)
{
    // Echo
    cout << "OnRecv Len = " << len << endl;
    cout << "Data = " << buffer << endl;

    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(len);
    BufferWriter bufferWriter(sendBuffer->GetData(), sendBuffer->GetFreeSize());
    bufferWriter.Write(buffer, len);
    sendBuffer->OnWrite(len);
    Send(sendBuffer);
}

void ServerSession::OnSend(int len)
{
    cout << "OnSend Len = " << len << endl;
}
