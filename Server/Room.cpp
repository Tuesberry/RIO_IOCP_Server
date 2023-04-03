#include "Room.h"
#include "Network/Packet.h"

Room gRoom;

Room::Room()
	: m_searchCnt(0)
	, m_changingLock()
	, m_players()
	, m_disconnectId()
	, m_connectId()
{
}

void Room::Login(std::shared_ptr<Player> player)
{
	{
		lock_guard<mutex> lock(m_changingLock);
		m_connectId.insert({ player->m_playerId, player });
	}
	if (m_searchCnt <= 0)
	{
		UpdatePlayers();
	}
}

void Room::Logout(std::shared_ptr<Player> player)
{
	{
		lock_guard<mutex> lock(m_changingLock);
		m_disconnectId.push_back(player->m_playerId);
		player->m_playerState = State::Disconnected;
		player->m_ownerSession = nullptr;
	}
	if (m_searchCnt <= 0)
	{
		UpdatePlayers();
	}
}

void Room::Logout(unsigned int userId)
{
	{
		lock_guard<mutex> lock(m_changingLock);
		m_disconnectId.push_back(userId);
		m_players.find(userId)->second->m_playerState = State::Disconnected;
		m_players.find(userId)->second->m_ownerSession = nullptr;
	}
	if (m_searchCnt <= 0)
	{
		UpdatePlayers();
	}
}

void Room::MovePlayer(unsigned int userId, unsigned short direction)
{
	// get player
	shared_ptr<Player> player = m_players.find(userId)->second;

	// find new direction
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

	// send move packet to me
	SendMoveMsg(userId, userId);

	// oldViewList & newViewList
	unordered_set<int> oldViewList = player->m_viewList;
	unordered_set<int> newViewList;

	// find new viewList
	m_searchCnt.fetch_add(1);

	int id = 0;
	shared_ptr<Player> targetPlayer = nullptr;

	map<unsigned int, std::shared_ptr<Player>>::iterator citer;
	for (citer = m_players.begin(); citer != m_players.end(); citer++)
	{
		id = citer->first;
		targetPlayer = citer->second;

		// self check
		if (id == userId)
			continue;

		// disconnect check
		if (targetPlayer->m_playerState == State::Disconnected)
			continue;

		// IsNear check
		if (IsNear(player->m_posX, player->m_posY, targetPlayer->m_posX, targetPlayer->m_posY))
			newViewList.insert(targetPlayer->m_playerId);
	}
	
	// update view list
	// new view list check
	unordered_set<int>::iterator viter;
	for (viter = newViewList.begin(); viter != newViewList.end(); viter++)
	{
		id = *viter;
		targetPlayer = m_players.find(id)->second;
		
		// check player viewlist
		if (oldViewList.find(id) == oldViewList.end())
		{
			// oldView에 없던 것
			SendEnterMsg(userId, id);
		}
		else
		{
			// 기존에 존재하던 것
			SendMoveMsg(userId, id);
		}

		// check targetPlayer viewlist
		if (targetPlayer->m_viewList.find(userId) == targetPlayer->m_viewList.end())
		{
			// targetPlayer의 viewlist에 없었음
			SendEnterMsg(id, userId);
		}
		else
		{
			// 안에 이미 있었음
			SendMoveMsg(id, userId);
		}
	}
	// old view list check
	for (viter = oldViewList.begin(); viter != oldViewList.end(); viter++)
	{
		if (newViewList.find(*viter) == newViewList.end())
		{
			// oldViewList에는 있는데, newViewlist에는 없음
			SendLeaveMsg(userId, *viter);
			SendLeaveMsg(*viter, userId);
		}
	}

	// player view list update
	player->m_viewList = newViewList;

	m_searchCnt.fetch_sub(1);

	// update player info if no one search playerInfo
	if (m_searchCnt <= 0)
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
	if (result < m_viewDistance)
		return true;
	return false;
}

void Room::SendMoveMsg(unsigned int userId, unsigned int targetId)
{
	shared_ptr<Player> player = m_players.find(userId)->second;
	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendMoveMsg(targetId, player->m_posX, player->m_posY);
	}
}

void Room::SendEnterMsg(unsigned int userId, unsigned int targetId)
{
	shared_ptr<Player> player = m_players.find(userId)->second;
	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendEnterMsg(targetId, player->m_posX, player->m_posY);
	}
}

void Room::SendLeaveMsg(unsigned int userId, unsigned int targetId)
{
	shared_ptr<Player> player = m_players.find(userId)->second;
	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendLeaveMsg(targetId);
	}
}

void Room::UpdatePlayers()
{
	lock_guard<mutex> lock(m_changingLock);
	// disconnect
	for (unsigned int& userId : m_disconnectId)
	{
		m_players.erase(userId);
	}
	m_disconnectId.clear();
	//connect
	map<unsigned int, std::shared_ptr<Player>>::iterator citer;
	for (citer = m_connectId.begin(); citer != m_connectId.end(); citer++)
	{
		citer->second->m_playerState = State::Connected;
		m_players.insert({ citer->first, citer->second });
	}
	m_connectId.clear();
}
