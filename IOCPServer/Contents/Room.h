#pragma once

#include "../IOCPServerCommon.h"

#include "Player.h"
#include "Zone.h"
#include "Sector.h"
#include "Job/JobQueue.h"
#include "Utils/LockMap.h"

class Room : public JobQueue
{
	friend class Zone;

public:
	enum{
		VIEW_DISTANCE = 500,
		MAP_WIDTH = 2000,
		MAP_HEIGHT = 2000,
		MAP_MIN_Y = -1000,
		MAP_MIN_X = -1000,
		ZONE_WIDTH = 500,
		ZONE_HEIGHT = 500,
		SECTOR_WIDTH = 250,
		SECTOR_HEIGHT = 250,
	};

public:
	Room();
	Room(const Room& other) = delete;
	Room(Room&& other) = delete;
	Room& operator=(const Room& other) = delete;
	Room& operator=(Room&& other) = delete;
	~Room() = default;

public:
	void Login(shared_ptr<Player> player);
	void Logout(shared_ptr<Player> player);
	void MovePlayer(shared_ptr<Player> player);

	int GetLoginCnt() { return m_loginCnt; }

	pair<int, int> GetPlayerZoneIdx(shared_ptr<Player> player);
	pair<int, int> GetPlayerSectorIdx(shared_ptr<Player> player);

	void GetAdjacentSectors(shared_ptr<Player> player, vector<shared_ptr<Sector>>& ret);
	
	atomic<int> m_sessionCnt = 0;

private:
	atomic<int> m_loginCnt;
	
	vector<vector<shared_ptr<Zone>>> m_zones;
	vector<vector<shared_ptr<Sector>>> m_sectors;

	LockMap<int, shared_ptr<Player>> m_players;

	int m_zoneListSizeY;
	int m_zoneListSizeX;
	int m_sectorListSizeY;
	int m_sectorListSizeX;
};

extern shared_ptr<Room> gRoom;