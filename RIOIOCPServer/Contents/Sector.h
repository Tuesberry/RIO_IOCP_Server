#pragma once

#include "../ServerCommon.h"

#include "Utils/LockUnorderedSet.h"
#include "Player.h"

/* --------------------------------------------------------
*	Class:		Sector
*	Summary:	Sector class
-------------------------------------------------------- */
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
	
	// sector에 속한 플레이어들
	LockUnorderedSet<shared_ptr<Player>> m_players;

	int m_idxY;
	int m_idxX;
};