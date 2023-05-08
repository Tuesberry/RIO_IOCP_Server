#pragma once

#include "Common.h"

#include "Player.h"
#include "Utils/LockUnorderedSet.h"

using LockSetPlayerRef = shared_ptr<LockUnorderedSet<shared_ptr<Player>>>;

class Room 
{
public:
	enum {
		VIEW_DISTANCE = 16,
		WAITING_TIME_LIMIT = 5,
		MAP_WIDTH = 800,
		MAP_HEIGHT = 600,
		ZONE_WIDTH = 20,
		ZONE_HEIGHT = 20
	};

public:
	Room();
	Room(const Room& other) = delete;
	Room(Room&& other) = delete;
	Room& operator=(const Room& other) = delete;
	Room& operator=(Room&& other) = delete;
	~Room() = default;

public:
	// Room Login & Logout
	void Login(std::shared_ptr<Player> player);
	void Logout(std::shared_ptr<Player> player);

	// Move Player & Synchronization
	void MovePlayer(std::shared_ptr<Player> player, unsigned short direction);

	int GetLoginCnt() { return m_loginCnt; }
	int GetUpdateMoveCnt() { return m_moveCnt; }

private:
	bool IsValidPlayer(shared_ptr<Player> player);
	tuple<int, int> GetPlayerZoneIdx(shared_ptr<Player> player);

	void SendMoveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer);
	void SendEnterMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer);
	void SendLeaveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer);

	void UpdatePlayerPosition(shared_ptr<Player> player, int direction);

private:
	atomic<int> m_moveCnt; 
	atomic<int> m_loginCnt;

	vector<vector<LockSetPlayerRef>> m_zones;
};

extern Room gRoom;