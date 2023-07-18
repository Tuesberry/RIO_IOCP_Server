#pragma once
#include "Common.h"

/* ----------------------------
*		SendBuffer
---------------------------- */

class SendBuffer : enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(int bufferSize);

	SendBuffer() = delete;
	SendBuffer(const SendBuffer& other) = delete;
	SendBuffer(SendBuffer&& other) = delete;
	SendBuffer& operator=(const SendBuffer& other) = delete;
	SendBuffer& operator=(SendBuffer&& other) = delete;
	~SendBuffer() = default;

	// get
	int GetDataSize() { return m_writePos; }
	int GetFreeSize() { return m_bufferSize - m_writePos; }

	// access data
	BYTE* GetData() { return &m_buffer[0]; }
	BYTE* GetWritePos() { return &m_buffer[m_writePos]; }

	// send & write data
	bool OnWrite(int numOfBytes);

private:
	int m_bufferSize;
	int m_writePos;
	vector<BYTE> m_buffer;
};