#pragma once

#include "../ServerCommon.h"
#include "Network/RioSession.h"

class Player; 
class PlayerInfo;

/* --------------------------------------------------------
*	class:		RioServerSession
*	Summary:	server session used for RIO
-------------------------------------------------------- */
class RioServerSession : public RioSession
{
public:
    RioServerSession();
    ~RioServerSession();

    virtual void OnRecvPacket(char* buffer, int len) override;
    virtual void OnSend(int len) override;
    virtual void OnDisconnected() override;

    void SendMoveMsg(int targetId, PlayerInfo& pInfo);
    void SendEnterMsg(int targetId, PlayerInfo& pInfo);
    void SendLeaveMsg(int targetId);
    void SendLoginResultMsg(bool result, int player_type);
    void SendMoveResultMsg();
    void SendChat(int targetId, string& playerStrId, string& chat);

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
