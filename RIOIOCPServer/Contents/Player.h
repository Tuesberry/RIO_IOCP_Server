#pragma once

#include "../ServerCommon.h"

#include "Thread/RWLock.h"

#ifdef IOCP
class IocpServerSession;
#endif
#ifdef RIO
class RioServerSession;
#endif

/* --------------------------------------------------------
*	Class:		State
*	Summary:	player state
-------------------------------------------------------- */
enum class State : unsigned short
{
	Connected,
	Disconnected
};

/* --------------------------------------------------------
*	Struct:		PlayerInfo
*	Summary:	player information
-------------------------------------------------------- */
struct PlayerInfo
{
	string playerId;
	int playerType;
	float x;
	float y;
	float z;
	float yaw;
	float pitch;
	float roll;
	float vx;
	float vy;
	float vz;
	float ax;
	float ay;
	float az;
	long long int timeStamp;
};

/* --------------------------------------------------------
*	Struct:		IdxInfo
*	Summary:	The section and zone the player belongs to
-------------------------------------------------------- */
struct IdxInfo
{
	int zone_y;
	int zone_x;
	int sector_y;
	int sector_x;
};

/* --------------------------------------------------------
*	Class:		Player
*	Summary:	player class 
-------------------------------------------------------- */
class Player
{
public:
#ifdef IOCP
	Player(int id, shared_ptr<IocpServerSession> session);
#endif
#ifdef RIO
	Player(int id, shared_ptr<RioServerSession> session);
#endif
	Player() = delete;
	Player(const Player& other) = delete;
	Player(Player&& other) = delete;
	Player& operator=(const Player& other) = delete;
	Player& operator=(Player&& other) = delete;
	~Player() = default;

	bool IsExistInViewList(int playerId);
	bool IsValidPlayer() { return m_playerState == State::Connected; }
	
	void SetViewList(unordered_set<int>& viewList);
	void SetInitialPos();

	unordered_set<int> GetViewList();

public:
	unsigned int m_playerId;

	PlayerInfo m_playerInfo;
	State m_playerState;
	IdxInfo m_idxInfo;

#ifdef IOCP
	shared_ptr<IocpServerSession> m_ownerSession;
#endif
#ifdef RIO
	shared_ptr<RioServerSession> m_ownerSession;
#endif

private:
	RWLock m_lock;
	unordered_set<int> m_viewList;
};