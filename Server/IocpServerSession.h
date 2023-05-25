#pragma once

#include "Common.h"

#if IOCP

#include "Network/IocpSession.h"

class Player;

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

    void SendMoveMsg(int targetId, unsigned short x, unsigned short y);
    void SendEnterMsg(int targetId, unsigned short x, unsigned short y);
    void SendLeaveMsg(int targetId);
    void SendLoginResult(bool result, unsigned short x, unsigned short y);

public:
    int m_connectClientId;
    int m_moveTime;
    int m_loginTime;
    int m_serverProcessTime;

    int m_sendTime;

    int m_updatePosTime;
    int m_synchronizePosTime;

    shared_ptr<Player> m_ownPlayer;
};
#endif // IOCP