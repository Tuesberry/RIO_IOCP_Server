#pragma once
#include "pch.h"
#include "CoreCommon.h"
#include "Network/IocpSession.h"

class ServerSession : public IocpSession
{
public:
    ServerSession();
    ~ServerSession();

    virtual void OnConnected() override;
    virtual void OnRecvPacket(BYTE* buffer, int len) override;
    virtual void OnSend(int len) override;
    virtual void OnDisconnected() override;

    void SendResult(bool isOk);
public:
};