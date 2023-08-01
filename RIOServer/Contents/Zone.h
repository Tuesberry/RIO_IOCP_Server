#pragma once

#include "../ServerCommon.h"

#include "Job/JobQueue.h"
#include "Player.h"

class Zone : public JobQueue
{
public:
	Zone(int idxY, int idxX);

	Zone() = delete;
	Zone(const Zone& other) = delete;
	Zone(Zone&& other) = delete;
	Zone& operator=(const Zone& other) = delete;
	Zone& operator=(Zone&& other) = delete;
	~Zone() = default;

	void MovePlayer(shared_ptr<Player> player);
	void Login(shared_ptr<Player> player);
	void Logout(shared_ptr<Player> player);
	void Chat(shared_ptr<Player> player, string chat);

private:
	bool IsNear(int posX1, int posY1, int posX2, int posY2, int viewDist);
	
	void SendMove(shared_ptr<Player> player, shared_ptr<Player> target);
	void SendEnter(shared_ptr<Player> player, shared_ptr<Player> target);
	void SendLeave(shared_ptr<Player> player, int targetId);
	void SendLeave(int playerId, int targetId);
	void SendMoveResult(shared_ptr<Player> player);
	void SendLoginResult(shared_ptr<Player> player);
	void SendChat(shared_ptr<Player> player, shared_ptr<Player> target, string& chat);

private:
	int m_idxX;
	int m_idxY;
};