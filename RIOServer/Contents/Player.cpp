#include "Player.h"
#include "Room.h"

Player::Player(int id, shared_ptr<RioServerSession> session)
	: m_playerId(id)
	, m_playerInfo()
	, m_playerState(State::Disconnected)
	, m_viewList()
	, m_ownerSession(session)
	, m_lock()
{
	SetInitialPos();
}

bool Player::IsExistInViewList(int playerId)
{
	ReadLockGuard lock(m_lock);
	return m_viewList.count(playerId);
}

void Player::SetViewList(unordered_set<int>& viewList)
{
	WriteLockGuard lock(m_lock);
	m_viewList = viewList;
}

void Player::SetInitialPos()
{
	::random_device rd;
	::mt19937 mt(rd());

	::uniform_int_distribution<int> xDist(0, Room::MAP_WIDTH - 1);
	::uniform_int_distribution<int> yDist(0, Room::MAP_HEIGHT - 1);

	m_playerInfo.x = Room::MAP_MIN_X + xDist(mt);
	m_playerInfo.y = Room::MAP_MIN_Y + yDist(mt);
	m_playerInfo.z = 210.0f;
}

unordered_set<int> Player::GetViewList()
{
	ReadLockGuard lock(m_lock);
	return m_viewList;
}