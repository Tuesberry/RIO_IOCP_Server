#pragma once
#include "pch.h"
#include "CoreCommon.h"
#include "Network/IocpSession.h"

class ServerSession : public IocpSession
{
public:
    ServerSession();
    ~ServerSession();

    virtual void OnRecvPacket(BYTE* buffer, int len) override;
    virtual void OnSend(int len) override;

    void SendResult(bool isOk);
public:
    int m_connectID;
    int m_posX;
    int m_posY;
};