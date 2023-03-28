#pragma once
#include "pch.h"
#include "CoreCommon.h"
#include "Network/IocpSession.h"

class ServerSession : public IocpSession
{
public:
    ~ServerSession();

    virtual void OnRecvPacket(BYTE* buffer, int len) override;
    virtual void OnSend(int len) override;

    void Broadcast();
};