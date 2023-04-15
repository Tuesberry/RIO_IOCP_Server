#pragma once

#include "pch.h"
#include "CoreCommon.h"

#include "Network/IocpSession.h"

/* --------------------------------------------------------
*	class:		ClientSession
*	Summary:	Session for Client
-------------------------------------------------------- */
class ClientSession : public IocpSession
{
public:
	ClientSession();
	~ClientSession();

	virtual void OnConnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int len) override;
	virtual void OnSend(int len) override;
	virtual void OnDisconnected() override;

	void SendLogin();
	void SendMove();

public:
	int m_sessionID;
	unsigned short m_posX;
	unsigned short m_posY;

	bool m_bLogin;
	bool m_bStartLogin;
	bool m_bConnect;
};