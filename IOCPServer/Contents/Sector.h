#pragma once

#include "../IOCPServerCommon.h"

#include "Utils/LockUnorderedSet.h"
#include "Player.h"

class Sector 
{
public:
	Sector(int idxY, int idxX);

	Sector() = delete;
	Sector(const Sector& other) = delete;
	Sector(Sector&& other) = delete;
	Sector& operator=(const Sector& other) = delete;
	Sector& operator=(Sector&& other) = delete;
	~Sector() = default;
	
	LockUnorderedSet<shared_ptr<Player>> m_players;

	int m_idxY;
	int m_idxX;
};