#pragma once
#include "Common.h"

/* ----------------------------
*		ProtoID
---------------------------- */
enum PROTO_ID : unsigned short
{
	LOGIN,
	C2S_MOVE,
	S2C_MOVE,
	S2C_ENTER,
	S2C_LEAVE,
	RESULT,
	LOGOUT
};

#pragma pack(1)
/* ----------------------------
*		PacketHeader
---------------------------- */
struct PacketHeader
{
	unsigned short size;
	unsigned short id; // protocol id 
};

/* ----------------------------
*		PKT_LOGIN
---------------------------- */
struct PKT_C2S_LOGIN
{
	PacketHeader header;
	int id;
};

/* ----------------------------
*		PKT_C2S_MOVE
---------------------------- */
struct PKT_C2S_MOVE
{
	PacketHeader header;
	int id;
	int direction;
};

/* ----------------------------
*		PKT_S2C_MOVE
---------------------------- */
struct PKT_S2C_MOVE
{
	PacketHeader header;
	int id;
	unsigned short x;
	unsigned short y;
	unsigned int moveTime; // delay 계산 용도
};

/* ----------------------------
*		PKT_S2C_ENTER
---------------------------- */
struct PKT_S2C_ENTER
{
	PacketHeader header;
	int id;
	unsigned short x;
	unsigned short y;
};

/* ----------------------------
*		PKT_S2C_LEAVE
---------------------------- */
struct PKT_S2C_LEAVE
{
	PacketHeader header;
	int id;
};

/* ----------------------------
*		PKT_RESULT
---------------------------- */
struct PKT_RESULT
{
	PacketHeader header;
	int id;
	bool result;
};

/* ----------------------------
*		PKT_C2S_LOGOUT
---------------------------- */
struct PKT_C2S_LOGOUT
{
	PacketHeader header;
	int id;
};

#pragma pack()
