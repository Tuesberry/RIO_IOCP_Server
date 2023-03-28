#pragma once
#include "Common.h"

/* ----------------------------
*		ProtoID
---------------------------- */
enum PROTO_ID : unsigned short
{
	TEST,
	LOGIN,
	INFO,
	RESULT
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
struct PKT_LOGIN
{
	PacketHeader header;
	int id;
};

/* ----------------------------
*		PKT_INFO
---------------------------- */
struct PKT_INFO
{
	PacketHeader header;
	int id;
	int posX;
	int poxY;
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

#pragma pack()
