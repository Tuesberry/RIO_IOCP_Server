#include "RoomOrigin.h"
#include "Network/Packet.h"

RoomOrigin gRoomOrigin;

RoomOrigin::RoomOrigin()
	: m_moveCnt(0)
	, m_players()
{
}

void RoomOrigin::Login(std::shared_ptr<Player> player)
{
	m_players.Insert(player->m_playerId, player);

	m_loginCnt.fetch_add(1);

	// set player state
	player->m_playerState = State::Connected;

	// send login result
	player->m_ownerSession->SendLoginResult(true, player->m_posX, player->m_posY);
}

void RoomOrigin::Logout(std::shared_ptr<Player> player)
{
	m_players.Erase(player->m_playerId);

	m_loginCnt.fetch_sub(1);
}

void RoomOrigin::MovePlayer(std::shared_ptr<Player> player, unsigned short direction)
{
	// get player
	if (IsValidPlayer(player) == false)
	{
		return;
	}

	// start processing time
	player->m_ownerSession->m_serverProcessTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

	// update time check
	int checkTime1 = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

	// UpdatePlayerPosition using direction
	UpdatePlayerPosition(direction, player);

	PlayerInfo pInfo{ player->m_posX, player->m_posY };
	m_playersInfo[player->m_playerId] = pInfo;

	// update check
	int checkTime2 = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
	player->m_ownerSession->m_updatePosTime = checkTime2 - checkTime1;

	// oldViewList & newViewList
	unordered_set<int> oldViewList = player->GetViewList();
	unordered_set<int> newViewList;

	// find new viewList
	FindNearPlayer(newViewList, player);

	// update view list
	// new view list check
	shared_ptr<Player> targetPlayer = nullptr;
	int id = 0;

	unordered_set<int>::iterator viter;
	for (viter = newViewList.begin(); viter != newViewList.end(); viter++)
	{
		id = *viter;
		{
			m_players.ReadLock();
			targetPlayer = m_players.m_map.find(id)->second;
			m_players.ReadUnlock();
		}

		// check player viewlist
		if (oldViewList.count(id))
		{
			// 기존에 존재하던 것
			SendMoveMsg(player, targetPlayer);
		}
		else
		{
			// oldView에 없던 것
			SendEnterMsg(player, targetPlayer);
		}

		// check targetPlayer viewlist
		if (targetPlayer->IsExistInViewList(player->m_playerId))
		{
			// 안에 이미 있었음
			SendMoveMsg(targetPlayer, player);
		}
		else
		{
			// targetPlayer의 viewlist에 없었음
			SendEnterMsg(targetPlayer, player);
		}
	}
	// old view list check
	for (viter = oldViewList.begin(); viter != oldViewList.end(); viter++)
	{
		if (newViewList.count(*viter) == false)
		{
			// oldViewList에는 있는데, newViewlist에는 없음
			{
				m_players.ReadLock();
				targetPlayer = m_players.m_map.find(*viter)->second;
				m_players.ReadUnlock();
			}

			SendLeaveMsg(player, targetPlayer);
			SendLeaveMsg(targetPlayer, player);
		}
	}

	player->m_ownerSession->m_synchronizePosTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - checkTime2;

	// player view list update
	player->SetViewList(newViewList);

	player->m_ownerSession->m_serverProcessTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - player->m_ownerSession->m_serverProcessTime;
	// move player
	SendMoveMsg(player, player);
}

bool RoomOrigin::IsNear(unsigned short posX1, unsigned short posY1, unsigned short posX2, unsigned short posY2)
{
	double result = sqrt(pow(posX1 - posX2, 2) + pow(posY1 - posY2, 2));
	if (result < VIEW_DISTANCE)
		return true;
	return false;
}

void RoomOrigin::SendMoveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer)
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

void RoomOrigin::SendEnterMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer)
{
	if (IsValidPlayer(player) == false || IsValidPlayer(targetPlayer) == false)
		return;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendEnterMsg(targetPlayer->m_playerId, targetPlayer->m_posX, targetPlayer->m_posY);
	}
}

void RoomOrigin::SendLeaveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer)
{
	if (IsValidPlayer(player) == false)
		return;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendLeaveMsg(targetPlayer->m_playerId);
	}
}

void RoomOrigin::UpdatePlayerPosition(int direction, shared_ptr<Player> player)
{
	switch (direction)
	{
	case MOVE_DIRECTION::FRONT:
		if (player->m_posY + 1 > MAP_HEIGHT)
			player->m_posY = 0;
		else
			player->m_posY += 1;
		break;
	case MOVE_DIRECTION::BACK:
		if (player->m_posY - 1 < 0)
			player->m_posY = MAP_HEIGHT;
		else
			player->m_posY -= 1;
		break;
	case MOVE_DIRECTION::RIGHT:
		if (player->m_posX + 1 > MAP_WIDTH)
			player->m_posX = 0;
		else
			player->m_posX += 1;
		break;
	case MOVE_DIRECTION::LEFT:
		if (player->m_posX - 1 < 0)
			player->m_posX = MAP_WIDTH;
		else
			player->m_posX -= 1;
		break;
	default:
		break;
	}
}

void RoomOrigin::FindNearPlayer(unordered_set<int>& viewList, shared_ptr<Player> player)
{
	int id = 0;
	shared_ptr<Player> targetPlayer = nullptr;

	map<unsigned int, std::shared_ptr<Player>>::iterator citer;
	
	m_players.ReadLock();
	for (citer = m_players.m_map.begin(); citer != m_players.m_map.end(); citer++)
	{
		id = citer->first;
		targetPlayer = citer->second;

		// self check
		if (id == player->m_playerId)
			continue;

		// disconnect check
		if (targetPlayer->m_playerState == State::Disconnected)
			continue;

		// IsNear check
		if (IsNear(player->m_posX, player->m_posY, targetPlayer->m_posX, targetPlayer->m_posY))
			viewList.insert(targetPlayer->m_playerId);
	}
	m_players.ReadUnlock();
}

bool RoomOrigin::IsValidPlayer(shared_ptr<Player> player)
{
	return player->m_playerState == State::Connected;
}