#pragma once

#include "Common.h"

/* --------------------------------------------------------
*	class:		IService
*	Summary:	service abstract class
-------------------------------------------------------- */
class IService
{
public:
	virtual bool Start(function<void()> logicFunc) abstract;
	virtual bool Stop() abstract;
};