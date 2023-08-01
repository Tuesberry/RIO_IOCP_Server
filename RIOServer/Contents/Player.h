#pragma once

#include "../ServerCommon.h"

#include "Thread/RWLock.h"
#include "../Session/RioServerSession.h"

enum class State : unsigned short
{
	Connected,
	Disconnected
};

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

struct IdxInfo
{
	int zone_y;
	int zone_x;
	int sector_y;
	int sector_x;
};

class Player
{
public:
	Player(int id, shared_ptr<RioServerSession> session);

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

	shared_ptr<RioServerSession> m_ownerSession;

private:
	RWLock m_lock;
	unordered_set<int> m_viewList;
};