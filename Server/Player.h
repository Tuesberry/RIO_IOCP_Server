#pragma once

#include "Common.h"

#include "Thread/RWLock.h"

#if IOCP
#include "IocpServerSession.h"
#else RIO
#include "RioServerSession.h"
#endif 

enum class State : unsigned short
{
	Connected,
	Disconnected
};

struct PlayerInfo
{
	unsigned short x;
	unsigned short y;
};

class Player
{
public:
#if IOCP
	Player(int id, shared_ptr<IocpServerSession> session);
	shared_ptr<IocpServerSession> m_ownerSession;
#else RIO
	Player(int id, shared_ptr<RioServerSession> session);
	shared_ptr<RioServerSession> m_ownerSession;
#endif

public:
	Player() = delete;
	Player(const Player& other) = delete;
	Player(Player&& other) = delete;
	Player& operator=(const Player& other) = delete;
	Player& operator=(Player&& other) = delete;
	~Player() = default;

	bool IsExistInViewList(int playerId);
	void SetViewList(unordered_set<int>& viewList);
	unordered_set<int> GetViewList();

public:
	void SetPlayerInitPos();

public:
	unsigned int m_playerId;
	unsigned short m_posX;
	unsigned short m_posY;
	State m_playerState;

private:
	RWLock m_lock;
	unordered_set<int> m_viewList;

};