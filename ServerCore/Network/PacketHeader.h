#pragma once

#pragma pack(1)
/* ----------------------------
*		PacketHeader
---------------------------- */
struct PacketHeader
{
	unsigned short size;
	unsigned short id; // protocol id 
};
#pragma pack()