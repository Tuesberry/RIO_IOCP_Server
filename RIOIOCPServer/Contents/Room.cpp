#include "Room.h"
#include "../Session/IocpServerSession.h"
#include "../Session/RioServerSession.h"

shared_ptr<Room> gRoom = make_shared<Room>();

/* --------------------------------------------------------
*	Method:		Room::Room
*	Summary:	Constructor
-------------------------------------------------------- */
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
			cout << i << " " << j << " CreateZone\n";
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

/* --------------------------------------------------------
*	Method:		Room::Login
*	Summary:	log in to the game room
*	Args:		shared_ptr<Player> player
*					player to join
-------------------------------------------------------- */
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
	player->m_ownerSession->SendLoginResultMsg(true, player->m_playerInfo.playerType);
}

/* --------------------------------------------------------
*	Method:		Room::Logout
*	Summary:	logout of the game room
*	Args:		shared_ptr<Player> player
*					player to logout
-------------------------------------------------------- */
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

/* --------------------------------------------------------
*	Method:		Room::MovePlayer
*	Summary:	move player in this game room
*	Args:		shared_ptr<Player> player
*					player to move
-------------------------------------------------------- */
void Room::MovePlayer(std::shared_ptr<Player> player)
{
	auto idx = GetPlayerZoneIdx(player);
	m_zones[idx.first][idx.second]->DoAsync(&Zone::MovePlayer, player);
}

/* --------------------------------------------------------
*	Method:		Room::Chat
*	Summary:	send chat to other players
*	Args:		shared_ptr<Player> player
*					player who sent the chat
*				string chat
*					chatting message
-------------------------------------------------------- */
void Room::Chat(shared_ptr<Player> player, string chat)
{
	auto idx = GetPlayerZoneIdx(player);
	m_zones[idx.first][idx.second]->DoAsync(&Zone::Chat, player, chat);
}

/* --------------------------------------------------------
*	Method:		Room::GetPlayerZoneIdx
*	Summary:	Returns the index of the zone 
*				where the player is located 
*	Args:		shared_ptr<Player> player
*					player to check
-------------------------------------------------------- */
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

/* --------------------------------------------------------
*	Method:		Room::GetPlayerSectorIdx
*	Summary:	Returns the index of the sector
*				where the player is located
*	Args:		shared_ptr<Player> player
*					player to check
-------------------------------------------------------- */
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

/* --------------------------------------------------------
*	Method:		Room::GetAdjacentSectors
*	Summary:	find adjacent sectors
*	Args:		shared_ptr<Player> player
*					player to check
*				vector<shared_ptr<Sector>>& ret
*					adjecent sectors
-------------------------------------------------------- */
void Room::GetAdjacentSectors(shared_ptr<Player> player, vector<shared_ptr<Sector>>& ret)
{
	int x = 0, y = 0;
	tie(y, x) = GetPlayerSectorIdx(player);
	
	for (int dy = 2; dy >= -2; dy--)
	{
		for (int dx = -2; dx <= 2; dx++)
		{
			int ny = y + dy;
			int nx = x + dx;
			if (nx < 0 || nx >= m_sectorListSizeX || ny < 0 || ny >= m_sectorListSizeY)
			{
				continue;
			}
			ret.push_back(m_sectors[ny][nx]);
		}
	}
}

/* --------------------------------------------------------
*	Method:		Room::GetNarrowAdjacentSectors
*	Summary:	find adjacent sectors(narrow version)
*	Args:		shared_ptr<Player> player
*					player to check
*				vector<shared_ptr<Sector>>& ret
*					adjecent sectors
-------------------------------------------------------- */
void Room::GetNarrowAdjacentSectors(shared_ptr<Player> player, vector<shared_ptr<Sector>>& ret)
{
	int x = 0, y = 0;
	tie(y, x) = GetPlayerSectorIdx(player);

	for (int dy = 1; dy >= -1; dy--)
	{
		for (int dx = -1; dx <= 1; dx++)
		{
			int ny = y + dy;
			int nx = x + dx;
			if (nx < 0 || nx >= m_sectorListSizeX || ny < 0 || ny >= m_sectorListSizeY)
			{
				continue;
			}
			ret.push_back(m_sectors[ny][nx]);
		}
	}
}
