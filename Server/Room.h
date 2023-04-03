#pragma once
#include "pch.h"
#include "CoreCommon.h"
#include "Player.h"

class Room
{
public:
	Room();
	Room(const Room& other) = delete;
	Room(Room&& other) = delete;
	Room& operator=(const Room& other) = delete;
	Room& operator=(Room&& other) = delete;
	~Room() = default;

public:
	void Login(std::shared_ptr<Player> player);
	void Logout(std::shared_ptr<Player> player);
	void Logout(unsigned int userId);
	void MovePlayer(unsigned int userId, unsigned short direction);

private:
	bool IsNear(unsigned int userId1, unsigned int userId2);
	bool IsNear(
		unsigned short posX1, 
		unsigned short posY1, 
		unsigned short posX2, 
		unsigned short posY2
	);

	void SendMoveMsg(unsigned int userId, unsigned int targetId);
	void SendEnterMsg(unsigned int userId, unsigned int targetId);
	void SendLeaveMsg(unsigned int userId, unsigned int targetId);

	void UpdatePlayers();

private:
	atomic<int> m_searchCnt;
	mutex m_changingLock;
	map<unsigned int, std::shared_ptr<Player>> m_players;

	// Room 접속 대기
	vector<unsigned int> m_disconnectId;
	map<unsigned int, std::shared_ptr<Player>> m_connectId;

	int m_viewDistance = 5;
};

extern Room gRoom;