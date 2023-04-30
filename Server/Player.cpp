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

void Player::SetPlayerInitPos()
{
	srand(time(NULL));
	m_posX = rand() % gRoom.MAP_WIDTH;
	m_posY = rand() % gRoom.MAP_HEIGHT;
}