#pragma once
#include "pch.h"
#include "CoreCommon.h"
#include "ServerSession.h"

enum class State : unsigned short
{
	Connected,
	Disconnected
};

class Player
{
public:
	Player() = delete;
	Player(int id, shared_ptr<ServerSession> session);

	Player(const Player& other) = delete;
	Player(Player&& other) = delete;
	Player& operator=(const Player& other) = delete;
	Player& operator=(Player&& other) = delete;
	~Player() = default;

private:
	void SetPlayerInitPos();

public:
	unsigned int m_playerId;
	unsigned short m_posX;
	unsigned short m_posY;
	unordered_set<int> m_viewList;

	State m_playerState;
	shared_ptr<ServerSession> m_ownerSession;
};