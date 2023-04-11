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

private:
	void AllocateBuffer();

private:
	char* m_buffer;
	int m_bufSize;
};