#include "Zone.h"
#include "Room.h"
#include "../Session/IocpServerSession.h"
#include "../Session/RioServerSession.h"

/* --------------------------------------------------------
*	Method:		Zone::Zone
*	Summary:	constructor
*	Args:		int idxY
*					zone's index y
*				int idxX
*					zone's index x
-------------------------------------------------------- */
Zone::Zone(int idxY, int idxX)
	: m_idxY(idxY)
	, m_idxX(idxX)
{

}

/* --------------------------------------------------------
*	Method:		Zone::MovePlayer
*	Summary:	move player and do movement synchronization
*	Args:		shared_ptr<Player> player
*					player to move
-------------------------------------------------------- */
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

			if (!IsNear(player->m_playerInfo.x, player->m_playerInfo.y, target->m_playerInfo.x, target->m_playerInfo.y, gRoom->VIEW_DISTANCE))
			{
				continue;
			}

			newViewList.insert(target->m_playerId);
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
			if (player->IsExistInViewList(target->m_playerId))
			{
				// send move
				// Player에게 target의 이동 정보를 전송하는 것
				// SendMove(player, target);
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
			SendLeave(target, player->m_playerId);
		}
	}

	// update player viewList
	player->SetViewList(newViewList);

	// process time
	player->m_ownerSession->m_serverProcessTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - player->m_ownerSession->m_serverProcessTime;

	// send player move
	SendMoveResult(player);
}

/* --------------------------------------------------------
*	Method:		Zone::Login
*	Summary:	login player
*	Args:		shared_ptr<Player> player
*					player to join
-------------------------------------------------------- */
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

/* --------------------------------------------------------
*	Method:		Zone::Logout
*	Summary:	logout player
*	Args:		shared_ptr<Player> player
*					player to logout
-------------------------------------------------------- */
void Zone::Logout(shared_ptr<Player> player)
{
	int y, x;
	tie(y, x) = gRoom->GetPlayerSectorIdx(player);
	gRoom->m_sectors[y][x]->m_players.Erase(player);
}

/* --------------------------------------------------------
*	Method:		Zone::Chat
*	Summary:	send chat to other players
*	Args:		shared_ptr<Player> player
*					player who sent the chat
*				string chat
*					chatting message
-------------------------------------------------------- */
void Zone::Chat(shared_ptr<Player> player, string chat)
{
	if (!player->IsValidPlayer())
	{
		return;
	}

	// get adjacent sectors
	vector<shared_ptr<Sector>> view;
	gRoom->GetNarrowAdjacentSectors(player, view);

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

			if (!IsNear(player->m_playerInfo.x, player->m_playerInfo.y, target->m_playerInfo.x, target->m_playerInfo.y, gRoom->VIEW_DISTANCE_NARROW))
			{
				continue;
			}

			SendChat(target, player, chat);
		}
		view[i]->m_players.ReadUnlock();
	}
}

/* --------------------------------------------------------
*	Method:		Zone::IsNear
*	Summary:	Check if two players are close 
*	Args:		int posX1
*					player1's position x
*				int posY1
*					player1's position y
*				int posX2
*					player2's position x
*				int posY2
*					player2's position y
*				int viewDist
*					referece distance
-------------------------------------------------------- */
bool Zone::IsNear(int posX1, int posY1, int posX2, int posY2, int viewDist)
{
	double result = sqrt(pow(posX1 - posX2, 2) + pow(posY1 - posY2, 2));
	if (result < viewDist)
	{
		return true;
	}

	return false;
}

/* --------------------------------------------------------
*	Method:		Zone::SendMove
*	Summary:	send movement information of targetplayer to player
*	Args:		shared_ptr<Player> player
*					Player to receive message 
*				shared_ptr<Player> target player
*					target player
-------------------------------------------------------- */
void Zone::SendMove(shared_ptr<Player> player, shared_ptr<Player> target)
{
	player->m_ownerSession->SendMoveMsg(target->m_playerId, target->m_playerInfo);
}

/* --------------------------------------------------------
*	Method:		Zone::SendEnter
*	Summary:	send movement information of targetplayer to player
*	Args:		shared_ptr<Player> player
*					Player to receive message
*				shared_ptr<Player> target player
*					target player
-------------------------------------------------------- */
void Zone::SendEnter(shared_ptr<Player> player, shared_ptr<Player> target)
{
	player->m_ownerSession->SendEnterMsg(target->m_playerId, target->m_playerInfo);
}

/* --------------------------------------------------------
*	Method:		Zone::SendLeave
*	Summary:	send information of targetplayer to player
*	Args:		shared_ptr<Player> player
*					Player to receive message
*				int targetId
*					target player to leave
-------------------------------------------------------- */
void Zone::SendLeave(shared_ptr<Player> player, int targetId)
{
	player->m_ownerSession->SendLeaveMsg(targetId);
}

/* --------------------------------------------------------
*	Method:		Zone::SendLeave
*	Summary:	send information of targetplayer to player
*	Args:		int playerId
*					Player to receive message
*				int targetId
*					target player to leave
-------------------------------------------------------- */
void Zone::SendLeave(int playerId, int targetId)
{
	if (gRoom->m_players.m_map.count(playerId))
	{
		auto pIter = gRoom->m_players.m_map.find(playerId);
		pIter->second->m_ownerSession->SendLeaveMsg(targetId);
	}
}

/* --------------------------------------------------------
*	Method:		Zone::SendMoveResult
*	Summary:	send movement information to player
*	Args:		shared_ptr<Player> player
*					player to receive message
-------------------------------------------------------- */
void Zone::SendMoveResult(shared_ptr<Player> player)
{
	player->m_ownerSession->SendMoveResultMsg();
}

/* --------------------------------------------------------
*	Method:		Zone::SendLoginResult
*	Summary:	send login information to player
*	Args:		shared_ptr<Player> player
*					player to receive message
-------------------------------------------------------- */
void Zone::SendLoginResult(shared_ptr<Player> player)
{
	player->m_ownerSession->SendLoginResultMsg(true, player->m_playerInfo.playerType);
}

/* --------------------------------------------------------
*	Method:		Zone::SendChat
*	Summary:	send chatting message to player
*	Args:		shared_ptr<Player> player
*					player to receive message
*				shared_ptr<Player> target
*					player who sent message
*				string& chat
*					message to send
-------------------------------------------------------- */
void Zone::SendChat(shared_ptr<Player> player, shared_ptr<Player> target, string& chat)
{
	player->m_ownerSession->SendChat(target->m_playerId, target->m_playerInfo.playerId, chat);
}
