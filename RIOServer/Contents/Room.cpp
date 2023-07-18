#include "Room.h"

shared_ptr<Room> gRoom = make_shared<Room>();

Room::Room()
	: m_loginCnt(0)
	, m_zones()
	, m_sectors()
	, m_zoneListSizeY(MAP_HEIGHT/ZONE_HEIGHT)
	, m_zoneListSizeX(MAP_WIDTH /ZONE_WIDTH)
	, m_sectorListSizeY(MAP_HEIGHT/SECTOR_HEIGHT)
	, m_sectorListSizeX(MAP_WIDTH/SECTOR_WIDTH)
{
	// init Zones
	for (int i = 0; i < MAP_HEIGHT / ZONE_HEIGHT; i++)
	{
		vector<shared_ptr<Zone>> v;
		for (int j = 0; j < MAP_WIDTH / ZONE_WIDTH; j++)
		{
			v.push_back(make_shared<Zone>(i, j));
		}
		m_zones.push_back(v);
	}
	// init Sectors
	for (int i = 0; i < MAP_HEIGHT / SECTOR_HEIGHT; i++)
	{
		vector<shared_ptr<Sector>> v;
		for (int j = 0; j < MAP_WIDTH / SECTOR_WIDTH; j++)
		{
			v.push_back(make_shared<Sector>(i, j));
		}
		m_sectors.push_back(v);
	}
}

void Room::Login(shared_ptr<Player> player)
{
	// get player sector point
	int y, x;
	tie(y, x) = GetPlayerSectorIdx(player);

	// increase login cnt
	m_loginCnt.fetch_add(1);

	// insert player
	m_sectors[y][x]->m_players.Insert(player);
	m_players.Insert(player->m_playerId, player);

	// set player state
	player->m_playerState = State::Connected;

	// set player idx;
	player->m_idxInfo.sector_x = x;
	player->m_idxInfo.sector_y = y;

	// send login result
	player->m_ownerSession->SendLoginResultMsg(true);
}

void Room::Logout(std::shared_ptr<Player> player)
{
	// erase player
	m_sectors[player->m_idxInfo.sector_y][player->m_idxInfo.sector_x]->m_players.Erase(player);
	m_players.Erase(player->m_playerId);

	// set player state
	player->m_playerState = State::Disconnected;

	// set login cnt
	m_loginCnt.fetch_sub(1);

	// TODO : Send Logout complete
}

void Room::MovePlayer(std::shared_ptr<Player> player)
{
	auto idx = GetPlayerZoneIdx(player);
	m_zones[idx.first][idx.second]->DoAsync(&Zone::MovePlayer, player);
}

pair<int, int> Room::GetPlayerZoneIdx(shared_ptr<Player> player)
{
	int y = (player->m_playerInfo.y - MAP_MIN_Y) / ZONE_HEIGHT;
	int x = (player->m_playerInfo.x - MAP_MIN_X) / ZONE_WIDTH;
	
	if (y < 0)
	{
		y = 0;
	}
	else if (y >= m_zoneListSizeY)
	{
		y = m_zoneListSizeY - 1;
	}
	if (x < 0)
	{
		x = 0;
	}
	else if (x >= m_zoneListSizeX)
	{
		x = m_zoneListSizeX - 1;
	}

	return { y, x };
}

pair<int, int> Room::GetPlayerSectorIdx(shared_ptr<Player> player)
{
	int y = (player->m_playerInfo.y - MAP_MIN_Y) / SECTOR_HEIGHT;
	int x = (player->m_playerInfo.x - MAP_MIN_X) / SECTOR_WIDTH;

	if (y < 0)
	{
		y = 0;
	}
	else if (y >= m_sectorListSizeY)
	{
		y = m_sectorListSizeY - 1;
	}
	if (x < 0)
	{
		x = 0;
	}
	else if (x >= m_sectorListSizeX)
	{
		x = m_sectorListSizeX - 1;
	}

	return { y, x };
}

void Room::GetAdjacentSectors(shared_ptr<Player> player, vector<shared_ptr<Sector>>& ret)
{
	const int dx[] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
	const int dy[] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };

	int x = 0, y = 0;
	tie(y, x) = GetPlayerSectorIdx(player);
	for (int i = 0; i < 9; i++)
	{
		int ny = y + dy[i];
		int nx = x + dx[i];
		if (nx < 0 || nx >= m_sectorListSizeX || ny < 0 || ny >= m_sectorListSizeY)
		{
			continue;
		}
		ret.push_back(m_sectors[ny][nx]);
	}
}
