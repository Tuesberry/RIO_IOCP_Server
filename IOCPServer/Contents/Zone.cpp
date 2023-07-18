#include "Zone.h"
#include "Room.h"

Zone::Zone(int idxY, int idxX)
	: m_idxY(idxY)
	, m_idxX(idxX)
{

}

void Zone::MovePlayer(shared_ptr<Player> player)
{
	if (!player->IsValidPlayer())
	{
		return;
	}

	// start processing time
	player->m_ownerSession->m_serverProcessTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

	// get current sector
	int y, x;
	tie(y, x) = gRoom->GetPlayerSectorIdx(player);

	// check same sector
	if (y != player->m_idxInfo.sector_y || x != player->m_idxInfo.sector_x)
	{
		// update sector
		gRoom->m_sectors[player->m_idxInfo.sector_y][player->m_idxInfo.sector_x]->m_players.Erase(player);
		gRoom->m_sectors[y][x]->m_players.Insert(player);

		// update player idx;
		player->m_idxInfo.sector_y = y;
		player->m_idxInfo.sector_x = x;
	}

	// get new viewlist
	unordered_set<int> newViewList;

	vector<shared_ptr<Sector>> view;
	gRoom->GetAdjacentSectors(player, view);

	// check view list -> sync
	for (int i = 0; i < view.size(); i++)
	{
		view[i]->m_players.ReadLock();
		for (auto target : view[i]->m_players.m_set)
		{
			if (!target->IsValidPlayer())
			{
				continue;
			}

			if (player->m_playerId == target->m_playerId)
			{
				continue;
			}

			if (!IsNear(player->m_playerInfo.x, player->m_playerInfo.y, target->m_playerInfo.x, target->m_playerInfo.y))
			{
				continue;
			}

			newViewList.insert(target->m_playerId);
			/*
			if (target->IsExistInViewList(player->m_playerId))
			{
				// send move
				SendMove(target, player);
			}
			else
			{
				// send enter
				SendEnter(target, player);
			}
			*/
			if (player->IsExistInViewList(target->m_playerId))
			{
				// send move
				SendMove(player, target);
			}
			else 
			{
				// send enter
				SendEnter(player, target);
			}
		}
		view[i]->m_players.ReadUnlock();
	}

	// check old view list
	for (auto target : player->GetViewList())
	{
		if (newViewList.count(target) == false)
		{
			// send leave message
			SendLeave(player, target);
			//SendLeave(target, player->m_playerId);
		}
	}

	// update player viewList
	player->SetViewList(newViewList);

	// process time
	player->m_ownerSession->m_serverProcessTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - player->m_ownerSession->m_serverProcessTime;

	// send player move
	SendMoveResult(player);
}

void Zone::Login(shared_ptr<Player> player)
{
	int y, x;
	tie(y, x) = gRoom->GetPlayerSectorIdx(player);
	gRoom->m_sectors[y][x]->m_players.Insert(player);

	gRoom->m_loginCnt.fetch_add(1);

	// set player state
	player->m_playerState = State::Connected;

	// set player idx;
	player->m_idxInfo.sector_x = x;
	player->m_idxInfo.sector_y = y;

	// send login result
	SendLoginResult(player);
}

void Zone::Logout(shared_ptr<Player> player)
{
	int y, x;
	tie(y, x) = gRoom->GetPlayerSectorIdx(player);
	gRoom->m_sectors[y][x]->m_players.Erase(player);
}

bool Zone::IsNear(int posX1, int posY1, int posX2, int posY2)
{
	double result = sqrt(pow(posX1 - posX2, 2) + pow(posY1 - posY2, 2));
	if (result < gRoom->VIEW_DISTANCE)
		return true;

	return false;
}

void Zone::SendMove(shared_ptr<Player> player, shared_ptr<Player> target)
{
	player->m_ownerSession->SendMoveMsg(target->m_playerId, target->m_playerInfo);
}

void Zone::SendEnter(shared_ptr<Player> player, shared_ptr<Player> target)
{
	player->m_ownerSession->SendEnterMsg(target->m_playerId, target->m_playerInfo);
}

void Zone::SendLeave(shared_ptr<Player> player, int targetId)
{
	player->m_ownerSession->SendLeaveMsg(targetId);
}

void Zone::SendLeave(int playerId, int targetId)
{
	auto pIter = gRoom->m_players.m_map.find(playerId);
	pIter->second->m_ownerSession->SendLeaveMsg(targetId);
}

void Zone::SendMoveResult(shared_ptr<Player> player)
{
	player->m_ownerSession->SendMoveResultMsg();
}

void Zone::SendLoginResult(shared_ptr<Player> player)
{
	player->m_ownerSession->SendLoginResultMsg(true);
}
