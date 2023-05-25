#include "Player.h"
#include "Room.h"

#if IOCP
Player::Player(int id, shared_ptr<IocpServerSession> session)
	: m_playerId(id)
	, m_posX(0)
	, m_posY(0)
	, m_viewList()
	, m_playerState(State::Disconnected)
	, m_ownerSession(session)
	, m_lock()
{
	SetPlayerInitPos();
}
#else RIO
Player::Player(int id, shared_ptr<RioServerSession> session)
	: m_playerId(id)
	, m_posX(0)
	, m_posY(0)
	, m_viewList()
	, m_playerState(State::Disconnected)
	, m_ownerSession(session)
{
	SetPlayerInitPos();
}
#endif 

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

void Player::SetPlayerInitPos()
{
	::random_device rd;
	::mt19937 mt(rd());

	::uniform_int_distribution<int> xDist(0, Room::MAP_WIDTH - 1);
	::uniform_int_distribution<int> yDist(0, Room::MAP_HEIGHT - 1);

	m_posX = xDist(mt);
	m_posY = yDist(mt);
}