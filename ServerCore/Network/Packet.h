#pragma once
#include "Common.h"

/* ----------------------------
*		ProtoID
---------------------------- */
enum PROTO_ID : unsigned short
{
	LOGIN,
	LOGIN_ADMIN,
	LOGIN_RESULT,
	LOGIN_RESULT_ADMIN,
	C2S_MOVE,
	S2C_MOVE,
	S2C_ENTER,
	S2C_LEAVE,
	A2S_REQUEST_PLAYER_INFO,
	S2A_PLAYER_INFO,
	RESULT,
	LOGOUT,
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
*		PKT_C2S_LOGIN
---------------------------- */
struct PKT_C2S_LOGIN
{
	PacketHeader header;
	int id;
	int loginTime;
};

/* ---------------------------------
*		PKT_C2S_LOGIN_ADMIN
--------------------------------- */
struct PKT_C2S_LOGIN_ADMIN
{
	PacketHeader header;
	int id;
};

/* ---------------------------------
*		PKT_S2C_LOGIN_RESULT
--------------------------------- */
struct PKT_S2C_LOGIN_RESULT
{
	PacketHeader header;
	int id;
	bool result;
	unsigned short x;
	unsigned short y;
	int loginTime;
};

/* ---------------------------------
*		PKT_S2C_LOGIN_RESULT_ADMIN
--------------------------------- */
struct PKT_S2C_LOGIN_RESULT_ADMIN
{
	PacketHeader header;
	int id;
	bool result;
};

/* ----------------------------
*		PKT_C2S_MOVE
---------------------------- */
enum MOVE_DIRECTION : unsigned short
{
	FRONT,
	RIGHT,
	LEFT,
	BACK
};

struct PKT_C2S_MOVE
{
	PacketHeader header;
	int id;
	unsigned short direction;
	int moveTime; // delay 계산 용도
};

/* ----------------------------
*		PKT_S2C_MOVE
---------------------------- */
struct PKT_S2C_MOVE
{
	PacketHeader header;
	int id;
	int targetId;
	unsigned short x;
	unsigned short y;
	int moveTime; // delay 계산 용도
	int processTime; // process time 계산
	int sendTime; // send delay 계산
	int recvTime; // recv delay 계산
	int updatePosTime;
	int synchronizePosTime;
};

/* ----------------------------
*		PKT_S2C_ENTER
---------------------------- */
struct PKT_S2C_ENTER
{
	PacketHeader header;
	int id;
	int targetId;
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
	int targetId;
};

/* ----------------------------
*		PKT_A2S_REQUEST_PLAYER_INFO
---------------------------- */
struct PKT_A2S_REQUEST_PLAYER_INFO
{
	PacketHeader header;
	int id;
};

/* ----------------------------
*		PKT_S2A_PLAYER_INFO
---------------------------- */
struct PKT_S2A_PLAYER_INFO
{
	PacketHeader header;
	int playerNum;
};

/* ----------------------------
*		PLAYER_INFO
---------------------------- */
struct PLAYER_INFO
{
	int playerId;
	unsigned short x;
	unsigned short y;
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
