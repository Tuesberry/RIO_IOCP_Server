#pragma once

/* ----------------------------
*		ProtoID
---------------------------- */
enum PROTO_ID : unsigned short
{
	C2S_LOGIN,
	C2S_LOGIN_ADMIN,
	S2C_LOGIN_RESULT,
	S2C_LOGIN_RESULT_ADMIN,
	C2S_MOVE,
	S2C_MOVE_RESULT,
	S2C_MOVE,
	S2C_ENTER,
	S2C_LEAVE,
	C2S_LOGOUT,
	S2C_LOGOUT_RESULT,
	C2S_CHAT,
	S2C_CHAT
};
