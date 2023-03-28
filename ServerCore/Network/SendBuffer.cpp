#include "SendBuffer.h"
SendBuffer::SendBuffer(int bufferSize)
	: m_bufferSize(bufferSize)
	, m_writePos(0)
	, m_buffer()
{
	m_buffer.resize(m_bufferSize);
}

bool SendBuffer::OnWrite(int numOfBytes)
{
	if (numOfBytes > GetFreeSize())
		return false;

	m_writePos += numOfBytes;
	return true;
}

