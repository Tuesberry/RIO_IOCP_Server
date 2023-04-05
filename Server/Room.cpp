#include "Room.h"
#include "Network/Packet.h"

Room gRoom;

Room::Room()
	: m_moveCnt(0)
	, m_playerLock()
	, m_players()
	, m_disconnectId()
	, m_connectId()
	, m_lastUpdateTime(high_resolution_clock::now())
{
}

void Room::Login(std::shared_ptr<Player> player)
{
	{
		lock_guard<mutex> lock(m_playerLock);
		m_connectId.insert({ player->m_playerId, player });
	}
	if (m_moveCnt <= 0 && m_bUpdate == false)
	{
		UpdatePlayers();
	}
}

void Room::Logout(std::shared_ptr<Player> player)
{
	{
		lock_guard<mutex> lock(m_playerLock);
		m_disconnectId.push_back(player->m_playerId);
		player->m_playerState = State::Disconnected;
		player->m_ownerSession = nullptr;
	}
	if (m_moveCnt <= 0 && m_bUpdate == false)
	{
		UpdatePlayers();
	}
}

void Room::Logout(unsigned int userId)
{
	{
		lock_guard<mutex> lock(m_playerLock);
		m_disconnectId.push_back(userId);
		m_players.find(userId)->second->m_playerState = State::Disconnected;
		m_players.find(userId)->second->m_ownerSession = nullptr;
	}
	if (m_moveCnt <= 0 && m_bUpdate == false)
	{
		UpdatePlayers();
	}
}

void Room::MovePlayer(unsigned int userId, unsigned short direction)
{
	// update 대기 수 너무 많거나, update 대기 시간이 길면 멈추기
	duration<double> sec = high_resolution_clock::now() - m_lastUpdateTime;
	while ((m_connectId.size() > 10) || (m_disconnectId.size() > 10) 
		|| ((sec.count() > WAITING_TIME_LIMIT) && ((m_connectId.empty() == false) || (m_disconnectId.empty() == false))))
	{
		this_thread::yield();
	}

	// start Move
	bool expected = false;	
	while (m_bUpdate.compare_exchange_weak(expected, true) == false)
	{
		expected = false;
	}
	m_moveCnt.fetch_add(1);
	m_bUpdate = false;

	// get player
	if(IsValidPlayer(userId) == false)
	{
		HandleError("MovePlayer_PlayerNonValid");
		m_moveCnt.fetch_sub(1);
		return;
	}

	shared_ptr<Player> player = m_players.find(userId)->second;

	// UpdatePlayerPosition using direction
	UpdatePlayerPosition(direction, player);

	// send move packet to me
	SendMoveMsg(userId, userId);

	// oldViewList & newViewList
	unordered_set<int> oldViewList = player->m_viewList;
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
		targetPlayer = m_players.find(id)->second;
		
		// check player viewlist
		if (oldViewList.count(id))
		{
			// 기존에 존재하던 것
			SendMoveMsg(userId, id);
		}
		else
		{
			// oldView에 없던 것
			SendEnterMsg(userId, id);
		}

		// check targetPlayer viewlist
		if (targetPlayer->m_viewList.count(userId))
		{
			// 안에 이미 있었음
			SendMoveMsg(id, userId);
		}
		else
		{
			// targetPlayer의 viewlist에 없었음
			SendEnterMsg(id, userId);
		}
	}
	// old view list check
	for (viter = oldViewList.begin(); viter != oldViewList.end(); viter++)
	{
		if (newViewList.count(*viter) == false)
		{
			// oldViewList에는 있는데, newViewlist에는 없음
			SendLeaveMsg(userId, *viter);
			SendLeaveMsg(*viter, userId);
		}
	}

	// player view list update
	player->m_viewList = newViewList;

	// moveCnt && update
	m_moveCnt.fetch_sub(1);
	if (m_moveCnt <= 0 && m_bUpdate == false)
	{
		UpdatePlayers();
	}

}

bool Room::IsNear(unsigned int userId1, unsigned int userId2)
{
	shared_ptr<Player> player1 = m_players.find(userId1)->second;
	shared_ptr<Player> player2 = m_players.find(userId2)->second;
	return IsNear(player1->m_posX, player1->m_posY, player2->m_posX, player2->m_posY);
}

bool Room::IsNear(unsigned short posX1, unsigned short posY1, unsigned short posX2, unsigned short posY2)
{
	double result = sqrt(pow(posX1 - posX2, 2) + pow(posY1 - posY2, 2));
	if (result < VIEW_DISTANCE)
		return true;
	return false;
}

void Room::SendMoveMsg(unsigned int userId, unsigned int targetId)
{
	if (IsValidPlayer(userId) == false)
		return;
	if (IsValidPlayer(targetId) == false)
		return SendLeaveMsg(userId, targetId);

	shared_ptr<Player> player = m_players.find(userId)->second;
	shared_ptr<Player> targetPlayer = m_players.find(targetId)->second;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendMoveMsg(targetId, targetPlayer->m_posX, targetPlayer->m_posY);
	}
}

void Room::SendEnterMsg(unsigned int userId, unsigned int targetId)
{
	if (IsValidPlayer(userId) == false || IsValidPlayer(targetId) == false)
		return;

	shared_ptr<Player> player = m_players.find(userId)->second;
	shared_ptr<Player> targetPlayer = m_players.find(targetId)->second;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendEnterMsg(targetId, targetPlayer->m_posX, targetPlayer->m_posY);
	}
}

void Room::SendLeaveMsg(unsigned int userId, unsigned int targetId)
{
	if (IsValidPlayer(userId) == false)
		return;

	shared_ptr<Player> player = m_players.find(userId)->second;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendLeaveMsg(targetId);
	}
}

void Room::UpdatePlayers()
{
	lock_guard<mutex> lock(m_playerLock);
	
	bool expected = false;
	if (m_bUpdate.compare_exchange_weak(expected, true) == false)
	{
		// 이미 bUpdate가 true인 경우, 이미 업데이트 중이므로, 종료
		return;
	}

	int expect = 0;
	while (m_moveCnt.compare_exchange_weak(expect, -1) == false)
	{
		// moveCnt가 0이 될 때까지 기다린다
		expect = 0;
	}

	// disconnect
	for (unsigned int& userId : m_disconnectId)
	{
		m_players.erase(userId);
	}
	m_disconnectId.clear();
	
	// connect
	map<unsigned int, std::shared_ptr<Player>>::iterator citer;
	for (citer = m_connectId.begin(); citer != m_connectId.end(); citer++)
	{
		// Player Connected in Room
		citer->second->m_playerState = State::Connected;
		m_players.insert({ citer->first, citer->second });

		// send Login OK
		citer->second->m_ownerSession->SendLoginResult(true, citer->second->m_posX, citer->second->m_posY);
	}
	m_connectId.clear();

	// reset bUpdate
	m_moveCnt.store(0);
	m_bUpdate.store(false);

	// time update
	m_lastUpdateTime = high_resolution_clock::now();
}

void Room::UpdatePlayerPosition(int direction, shared_ptr<Player> player)
{
	switch (direction)
	{
	case MOVE_DIRECTION::FRONT:
		player->m_posY += 1;
		break;
	case MOVE_DIRECTION::BACK:
		player->m_posY -= 1;
		break;
	case MOVE_DIRECTION::RIGHT:
		player->m_posX += 1;
		break;
	case MOVE_DIRECTION::LEFT:
		player->m_posX -= 1;
		break;
	default:
		break;
	}
}

void Room::FindNearPlayer(unordered_set<int>& viewList, shared_ptr<Player> player)
{
	int id = 0;
	shared_ptr<Player> targetPlayer = nullptr;

	map<unsigned int, std::shared_ptr<Player>>::iterator citer;
	for (citer = m_players.begin(); citer != m_players.end(); citer++)
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
}

bool Room::IsValidPlayer(unsigned int userId)
{
	// check valid
	if (m_players.count(userId) == 0)
	{
		return false;
	}
	if (m_players.find(userId)->second->m_playerState == State::Disconnected)
	{
		return false;
	}
	if (m_players.find(userId)->second->m_ownerSession == nullptr)
	{
		return false;
	}

	return true;
}
