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
	srand(time(NULL));
	m_posX = rand() % 50;//gRoom.MAP_WIDTH;
	m_posY = rand() % 50;//gRoom.MAP_HEIGHT;
}