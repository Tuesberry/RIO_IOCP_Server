#pragma once
#include "Common.h"

/* ----------------------------
*		RioBuffer
---------------------------- */

class RioBuffer
{
public:
	RioBuffer(int bufferSize);

	RioBuffer() = delete;
	RioBuffer(const RioBuffer& other) = delete;
	RioBuffer(RioBuffer&& other) = delete;
	RioBuffer& operator=(const RioBuffer& other) = delete;
	RioBuffer& operator=(RioBuffer&& other) = delete;
	~RioBuffer();

	char* GetBuffer() { return m_buffer; }
	int GetBufferSize() { return m_bufSize; }

	char* GetWritePos() { return &m_buffer[m_writePos]; }
	char* GetReadPos() { return &m_buffer[m_readPos]; }

	bool OnWriteBuffer(int writeSize);
	bool OnReadBuffer(int readSize);

private:
	void AllocateBuffer();

private:
	char* m_buffer;
	int m_bufSize;

	int m_writePos;
	int m_readPos;
};