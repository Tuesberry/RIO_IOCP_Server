#pragma once

#include "Common.h"

/* --------------------------------------------------------
*	class:		RingBuffer
*	Summary:	ring buffer
-------------------------------------------------------- */
class RingBuffer
{
	/*
	*	2 case 
	*	//----tail====head----//
	*	//====head----tail====//
	* 
	*	head = write position
	*	tail = read position
	*	end = capacity
	*/
public:
	RingBuffer(char* buffer, int capacity);

	RingBuffer() = delete;
	RingBuffer(const RingBuffer& other) = delete;
	RingBuffer(RingBuffer&& other) = delete;
	RingBuffer& operator=(const RingBuffer& other) = delete;
	RingBuffer& operator=(RingBuffer&& other) = delete;
	~RingBuffer();

	int GetDataSize();
	int GetFreeSize();

	bool WriteBuffer(char* data, int writeSize);
	bool ReadBuffer(char* destData, int readSize);

	int GetWriteOffset() { return m_headPos; }
	int GetReadOffset() { return m_tailPos; }

	bool OnWriteBuffer(int writeSize);
	bool OnReadBuffer(int readSize);

	char* GetBuffer() { return m_buffer; }
	char* GetWriteBuf() { return &m_buffer[m_headPos]; }
	char* GetReadBuf() { return &m_buffer[m_tailPos]; }

	int GetChunkWriteSize();
	int GetChunkReadSize();

protected:
	int m_capacity;
	char* m_buffer;

	int m_headPos;
	int m_tailPos;
};