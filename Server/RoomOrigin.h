#pragma once

#include "Common.h"

#include "Player.h"
#include "Utils/LockMap.h"

class RoomOrigin
{
public:
	enum {
		VIEW_DISTANCE = 16,
		WAITING_TIME_LIMIT = 5,
		MAP_WIDTH = 800,
		MAP_HEIGHT = 600
	};

public:
	RoomOrigin();
	RoomOrigin(const RoomOrigin& other) = delete;
	RoomOrigin(RoomOrigin&& other) = delete;
	RoomOrigin& operator=(const RoomOrigin& other) = delete;
	RoomOrigin& operator=(RoomOrigin&& other) = delete;
	~RoomOrigin() = default;

public:
	void Login(std::shared_ptr<Player> player);
	void Logout(std::shared_ptr<Player> player);

	void MovePlayer(std::shared_ptr<Player> player, unsigned short direction);

	int GetLoginCnt() { return m_loginCnt; }
	int GetUpdateMoveCnt() { return m_moveCnt; }

private:
	bool IsNear(
		unsigned short posX1,
		unsigned short posY1,
		unsigned short posX2,
		unsigned short posY2
	);

	void SendMoveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer);
	void SendEnterMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer);
	void SendLeaveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer);

	void UpdatePlayerPosition(int direction, shared_ptr<Player> player);

	void FindNearPlayer(unordered_set<int>& viewList, shared_ptr<Player> player);

	bool IsValidPlayer(shared_ptr<Player> player);

private:
	atomic<int> m_moveCnt;
	atomic<int> m_loginCnt;

	LockMap<unsigned int, std::shared_ptr<Player>> m_players;

public:
	map<int, PlayerInfo> m_playersInfo;
};

extern RoomOrigin gRoomOrigin;