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

public:
	void Debug()
	{
		lock_guard<mutex> lock(m_debugLock);
		for (auto text : m_debug)
		{
			cout << text << endl;
		}
	}
	void SaveDebug(string main)
	{
		lock_guard<mutex> lock(m_debugLock);
		m_debug.push_back(ThreadId +" | Method = " + main + " HeadPos = " + to_string(m_headPos) + " TailPos = " + to_string(m_tailPos));
	}
	vector<string> m_debug;
	mutex m_debugLock;

protected:
	int m_capacity;
	char* m_buffer;

	int m_headPos;
	int m_tailPos;
};