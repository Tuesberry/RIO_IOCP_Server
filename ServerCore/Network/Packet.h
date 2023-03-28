#pragma once
#include "Common.h"

/* ----------------------------
*		ProtoID
---------------------------- */
enum PROTO_ID : unsigned short
{
	TEST,

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
*		PKT_TEST
---------------------------- */
struct PKT_TEST
{
	PacketHeader header;
};
#pragma pack()
