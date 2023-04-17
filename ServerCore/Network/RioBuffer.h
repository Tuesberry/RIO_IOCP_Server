#pragma once
#include "Common.h"

/* --------------------------------------------------------
*	class:		RioBuffer
*	Summary:	buffer used in rio server
-------------------------------------------------------- */
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
	int GetFreeSize() { return m_bufSize - m_writePos; }
	int GetDataSize() { return m_writePos - m_readPos; }

	int GetWritePos() { return m_writePos; }
	int GetReadPos() { return m_readPos; }

	char* GetWriteBuf() { return &m_buffer[m_writePos]; }
	char* GetReadBuf() { return &m_buffer[m_readPos]; }

	bool OnWriteBuffer(int writeSize);
	bool OnReadBuffer(int readSize);

	void AdjustPos();

private:
	void AllocateBuffer();

private:
	char* m_buffer;
	int m_bufSize;

	int m_writePos;
	int m_readPos;
};