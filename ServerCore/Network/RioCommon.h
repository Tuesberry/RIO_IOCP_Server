#pragma once

enum RioConfig
{
	MAX_RIO_RESULT = 256,
	MAX_CQ_SIZE = 327680,
	BUFFER_SIZE = 65536,
	MAX_RECV_RQ_SIZE = 32,
	MAX_SEND_RQ_SIZE = 128,
	RECV_BUFF_COUNT = 1,
	SEND_BUFF_COUNT = 1,

	SEND_LIMIT = 400,
	COMMIT_TIME = 20,
	MAX_POST_CNT = 128,
};