#pragma once
#include "CoreCommon.h"
#include "pch.h"
#include "Network/IocpSession.h"

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

	unsigned int m_moveTime;
	bool m_bAddDelay;
};