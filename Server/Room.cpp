#include "Room.h"
#include "Network/Packet.h"

shared_ptr<Room> gRoom = make_shared<Room>();

Room::Room()
	: m_moveCnt(0)
	, m_playerLock()
	, m_players()
	, m_updateCnt(0)
	, m_zones(MAP_WIDTH/ZONE_WIDTH, vector<unordered_set<shared_ptr<Player>>>(MAP_HEIGHT / ZONE_HEIGHT))
{

}

void Room::Login(std::shared_ptr<Player> player)
{
	// insert player
	auto pos = GetPlayerZoneIdx(player);
	m_zones[std::get<0>(pos)][std::get<1>(pos)].insert(player);

	// set player state
	player->m_playerState = State::Connected;

	// send login result
	player->m_ownerSession->SendLoginResult(true, player->m_posX, player->m_posY);
}

void Room::Logout(std::shared_ptr<Player> player)
{
	// erase player
	auto pos = GetPlayerZoneIdx(player);
	m_zones[std::get<0>(pos)][std::get<1>(pos)].erase(player);
}

void Room::MovePlayer(std::shared_ptr<Player> player, unsigned short direction)
{
	if (IsValidPlayer(player) == false)
		return;

	// start processing time
	player->m_ownerSession->m_serverProcessTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

	// save beforePos
	auto beforePos = GetPlayerZoneIdx(player);
	int beforeX = std::get<0>(beforePos);
	int beforeY = std::get<1>(beforePos);

	// update new Position
	UpdatePlayerPosition(player, direction);
	auto newPos = GetPlayerZoneIdx(player);
	int newX = std::get<0>(newPos);
	int newY = std::get<1>(newPos);

	// check same zone
	if (beforeX != newX || beforeY != newY)
	{
		// update zone
		m_zones[newX][newY].insert(player);
		m_zones[beforeX][beforeY].erase(player);
	}
	
	// new view list
	unordered_set<int> newViewList;

	// synchronization
	for (int x = -1; x < 2; x++)
	{
		for (int y = -1; y < 2; y++)
		{
			int zoneX = newX + x;
			int zoneY = newY + y;
			// check map
			if (zoneX < 0 || zoneX >= MAP_WIDTH / ZONE_WIDTH || zoneY < 0 || zoneY >= MAP_HEIGHT / ZONE_HEIGHT)
			{
				continue;
			}
			// iterate
			for (auto iter = m_zones[zoneX][zoneY].begin(); iter != m_zones[zoneX][zoneY].end(); iter++)
			{
				// check invalid
				if (IsValidPlayer(*iter) == false)
					continue;
				// check same
				if ((*iter)->m_playerId == player->m_playerId)
					continue;

				// add in view list
				newViewList.insert((*iter)->m_playerId);

				// send move packet
				// check target viewlist
				if ((*iter)->m_viewList.count(player->m_playerId))
				{
					// 기존에 존재하던 것
					SendMoveMsg(*iter, player);
				}
				else
				{
					// 새로 추가된 것
					(*iter)->m_viewList.insert(player->m_playerId);
					SendEnterMsg(*iter, player);
				}
				// check player viewlist
				if (player->m_viewList.count((*iter)->m_playerId))
				{
					// 기존에 존재하던 것
					SendMoveMsg(player, *iter);
				}
				{
					// 새로 추가된 것
					SendEnterMsg(player, *iter);
				}
			}
		}
	}

	// update player view list
	player->m_viewList = newViewList;

	// send player move
	SendMoveMsg(player, player);
}

bool Room::IsValidPlayer(shared_ptr<Player> player)
{
	return player->m_playerState == State::Connected;
}

tuple<int, int> Room::GetPlayerZoneIdx(shared_ptr<Player> player)
{
	return tuple<int, int>(player->m_posX / ZONE_WIDTH, player->m_posY / ZONE_HEIGHT);
}

void Room::SendMoveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer)
{
	if (IsValidPlayer(player) == false)
		return;
	if (IsValidPlayer(targetPlayer) == false)
		return;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendMoveMsg(targetPlayer->m_playerId, targetPlayer->m_posX, targetPlayer->m_posY);
	}
}

void Room::SendEnterMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer)
{
	if (IsValidPlayer(player) == false || IsValidPlayer(targetPlayer) == false)
		return;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendEnterMsg(targetPlayer->m_playerId, targetPlayer->m_posX, targetPlayer->m_posY);
	}
}

void Room::SendLeaveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer)
{
	if (IsValidPlayer(player) == false)
		return;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendLeaveMsg(targetPlayer->m_playerId);
	}
}

void Room::UpdatePlayerPosition(shared_ptr<Player> player, int direction)
{
	switch (direction)
	{
	case MOVE_DIRECTION::FRONT:
		if (player->m_posY + 1 >= MAP_HEIGHT)
			player->m_posY = 0;
		else
			player->m_posY += 1;
		break;
	case MOVE_DIRECTION::BACK:
		if (player->m_posY - 1 <= 0)
			player->m_posY = MAP_HEIGHT -1;
		else
			player->m_posY -= 1;
		break;
	case MOVE_DIRECTION::RIGHT:
		if (player->m_posX + 1 >= MAP_WIDTH)
			player->m_posX = 0;
		else
			player->m_posX += 1;
		break;
	case MOVE_DIRECTION::LEFT:
		if (player->m_posX - 1 <= MAP_WIDTH)
			player->m_posX = MAP_WIDTH - 1;
		else
			player->m_posX -= 1;
		break;
	default:
		break;
	}
}
