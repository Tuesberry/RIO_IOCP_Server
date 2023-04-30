#pragma once

#include "Common.h"

#include "Player.h"
#include "JobQueue.h"

class Room : public JobQueue
{
public:
	enum {
		VIEW_DISTANCE = 16,
		WAITING_TIME_LIMIT = 5,
		MAP_WIDTH = 800,
		MAP_HEIGHT = 600,
		ZONE_WIDTH = 10,
		ZONE_HEIGHT = 10
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

	int GetLoginCnt() { return m_players.size(); }
	int GetUpdateMoveCnt() { return m_moveCnt; }

private:
	bool IsValidPlayer(shared_ptr<Player> player);
	tuple<int, int> GetPlayerZoneIdx(shared_ptr<Player> player);

	void SendMoveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer);
	void SendEnterMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer);
	void SendLeaveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer);

	void UpdatePlayerPosition(shared_ptr<Player> player, int direction);

public:
	long long int m_updateCnt;

private:
	atomic<int> m_moveCnt; 
	atomic<bool> m_bUpdate;

	mutex m_playerLock;
	map<unsigned int, std::shared_ptr<Player>> m_players;

	vector<vector<unordered_set<shared_ptr<Player>>>> m_zones;
};

extern shared_ptr<Room> gRoom;