#pragma once
#include "Common.h"

/* ----------------------------
*		RecvBuffer
---------------------------- */

class RecvBuffer
{
public:
	RecvBuffer(int bufferSize);

	RecvBuffer() = delete;
	RecvBuffer(const RecvBuffer& other) = delete;
	RecvBuffer(RecvBuffer&& other) = delete;
	RecvBuffer& operator=(const RecvBuffer& other) = delete;
	RecvBuffer& operator=(RecvBuffer&& other) = delete;
	~RecvBuffer() = default;

	// get
	int GetDataSize() { return m_writePos - m_readPos; }
	int GetFreeSize() { return m_capacity - m_writePos; }

	// access data
	BYTE* GetReadPos() { return &m_buffer[m_readPos]; }
	BYTE* GetWritePos() { return &m_buffer[m_writePos]; }

	// read & write data
	bool OnRead(int numOfBytes);
	bool OnWrite(int numOfBytes);
	
	// read & write pos adjust
	void AdjustPos();

private:
	int m_capacity;
	int m_bufferSize;
	int m_readPos;
	int m_writePos;

	vector<BYTE> m_buffer;
};