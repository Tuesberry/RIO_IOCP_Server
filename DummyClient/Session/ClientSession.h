#pragma once

#include "../DummyClientCommon.h"
#include "Network/IocpSession.h"

/* --------------------------------------------------------
*	struct:		PositionInfo
*	Summary:	position info for Client
-------------------------------------------------------- */
struct PositionInfo
{
	float x;
	float y;
	float z;
	float yaw;
	float pitch;
	float roll;
	float vx;
	float vy;
	float vz;
	float ax;
	float ay;
	float az;
};

/* --------------------------------------------------------
*	class:		MoveComponent
*	Summary:	component handling move
-------------------------------------------------------- */
class MoveComponent
{
public:
	MoveComponent() = default;
	~MoveComponent() = default;

	PositionInfo m_positionInfo;
	float m_speed = 600.0f;

	void Update(float deltaTime);

private:
	pair<int, int> GetNewDirection();
	void UpdateVelocityByNewDirection();
	void UpdateNewPosition(float deltaTime);
};

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
	MoveComponent m_moveComp;
	bool m_bLogin;
	bool m_bStartLogin;
	bool m_bConnect;
};