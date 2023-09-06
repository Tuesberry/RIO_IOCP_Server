#include "Player.h"
#include "Room.h"
#include "../Session/IocpServerSession.h"
#include "../Session/RioServerSession.h"

/* --------------------------------------------------------
*	Method:		Player::Player
*	Summary:	Constructor
*	Args:		int id
*					player id
*				shared_ptr<IocpServerSession> session
*					session of player
-------------------------------------------------------- */
#ifdef IOCP
Player::Player(int id, shared_ptr<IocpServerSession> session)
	: m_playerId(id)
	, m_playerInfo()
	, m_playerState(State::Disconnected)
	, m_viewList()
	, m_ownerSession(session)
	, m_lock()
{
	SetInitialPos();
}
#endif

/* --------------------------------------------------------
*	Method:		Player::Player
*	Summary:	Constructor
*	Args:		int id
*					player id
*				shared_ptr<RioServerSession> session
*					session of player
-------------------------------------------------------- */
#ifdef RIO
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
#endif

/* --------------------------------------------------------
*	Method:		Player::IsExistInViewList
*	Summary:	Check whether it exists 
*				in the player's view list
*	Args:		int playerId
*					player id to check
-------------------------------------------------------- */
bool Player::IsExistInViewList(int playerId)
{
	ReadLockGuard lock(m_lock);
	return m_viewList.count(playerId);
}

/* --------------------------------------------------------
*	Method:		Player::SetViewList
*	Summary:	set view list
*	Args:		unordered_set<int>& viewList
-------------------------------------------------------- */
void Player::SetViewList(unordered_set<int>& viewList)
{
	WriteLockGuard lock(m_lock);
	m_viewList = viewList;
}

/* --------------------------------------------------------
*	Method:		Player::SetInitialPos
*	Summary:	set initial position of player(for test)
-------------------------------------------------------- */
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

/* --------------------------------------------------------
*	Method:		Player::GetViewList
*	Summary:	return view list of player
-------------------------------------------------------- */
unordered_set<int> Player::GetViewList()
{
	ReadLockGuard lock(m_lock);
	return m_viewList;
}