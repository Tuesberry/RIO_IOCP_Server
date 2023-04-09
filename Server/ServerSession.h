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
    virtual void OnDisconnected() override;

    void SendMoveMsg(int targetId, unsigned short x, unsigned short y);
    void SendEnterMsg(int targetId, unsigned short x, unsigned short y);
    void SendLeaveMsg(int targetId);
    void SendLoginResult(bool result, unsigned short x, unsigned short y);

public:
    int m_connectClientId;
    int m_moveTime;
    int m_loginTime;
    int m_serverProcessTime;
};