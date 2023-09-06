#pragma once

#include "Common.h"

#include "RingBuffer.h"

/* --------------------------------------------------------
*	class:		NetBuffer
*	Summary:	ring buffer class used by IOCP
-------------------------------------------------------- */
class NetBuffer : public RingBuffer
{
public:
	NetBuffer(int bufferSize);

	NetBuffer() = delete;
	NetBuffer(const NetBuffer& other) = delete;
	NetBuffer(NetBuffer&& other) = delete;
	NetBuffer& operator=(const NetBuffer& other) = delete;
	NetBuffer& operator=(NetBuffer&& other) = delete;
	~NetBuffer();

private:
	vector<char> m_bufferVec;
};