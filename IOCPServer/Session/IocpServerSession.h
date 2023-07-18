#pragma once

#include "../IOCPServerCommon.h"
#include "Network/IocpSession.h"

class Player;
class PlayerInfo;

/* --------------------------------------------------------
*	class:		IocpServerSession
*	Summary:	server session used for IOCP
-------------------------------------------------------- */

class IocpServerSession : public IocpSession
{
public:
    IocpServerSession();
    ~IocpServerSession();

    virtual void OnRecvPacket(BYTE* buffer, int len) override;
    virtual void OnSend(int len) override;
    virtual void OnDisconnected() override;

    void SendMoveMsg(int targetId, PlayerInfo& pInfo);
    void SendEnterMsg(int targetId, PlayerInfo& pInfo);
    void SendLeaveMsg(int targetId);
    void SendLoginResultMsg(bool result);
    void SendMoveResultMsg();

public:
    string m_playerId;
    int m_sessionId;

    int m_moveTime;
    int m_loginTime;
    int m_serverProcessTime;
    int m_sendTime;

    bool m_bRequestResult;

    shared_ptr<Player> m_ownPlayer;
};
