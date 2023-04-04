#include "Player.h"

Player::Player(int id, shared_ptr<ServerSession> session)
	: m_playerId(id)
	, m_posX(rand()%400)
	, m_posY(rand()%300)
	, m_viewList()
	, m_playerState(State::Disconnected)
	, m_ownerSession(session)
{
}
