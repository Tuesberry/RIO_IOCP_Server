#include "RioBuffer.h"

RioBuffer::RioBuffer(int bufferSize)
	: m_buffer(nullptr)
	, m_bufSize(bufferSize)
	, m_writePos(0)
	, m_readPos(0)
{
	AllocateBuffer();
}

RioBuffer::~RioBuffer()
{
	// free
	VirtualFreeEx(GetCurrentProcess(), m_buffer, 0, MEM_RELEASE);
}

bool RioBuffer::OnWriteBuffer(int writeSize)
{
	if (writeSize > GetFreeSize())
	{
		HandleError("OnWriteBuffer");
		return false;
	}

	m_writePos += writeSize;

	return true;
}

bool RioBuffer::OnReadBuffer(int readSize)
{
	if (readSize > GetDataSize())
	{
		HandleError("OnReadBuffer");
		return false;
	}

	m_readPos += readSize;

	return true;
}

void RioBuffer::AdjustPos()
{
	if (m_writePos == m_readPos)
	{
		m_writePos = 0;
		m_readPos = 0;
	}
	else if(GetFreeSize() < m_bufSize / 2)
	{
		int dataSize = GetDataSize();
		memcpy(m_buffer, &m_buffer[m_readPos], dataSize);
		m_writePos = dataSize;
		m_readPos = 0;
	}
}

void RioBuffer::AllocateBuffer()
{
	// alloc
	m_buffer = reinterpret_cast<char*>(VirtualAllocEx(GetCurrentProcess(), 0, m_bufSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	if (m_buffer == nullptr)
	{
		HandleError("AllocateBuffer");
		return;
	}
}
