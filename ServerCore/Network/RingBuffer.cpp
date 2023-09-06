#include "RingBuffer.h"

/* --------------------------------------------------------
*	Method:		RingBuffer::RingBuffer
*	Summary:	constructor
-------------------------------------------------------- */
RingBuffer::RingBuffer(char* buffer, int capacity)
	: m_buffer(buffer)
	, m_capacity(capacity)
	, m_size(0)
	, m_headPos(0)
	, m_tailPos(0)
{
}

/* --------------------------------------------------------
*	Method:		RingBuffer::~RingBuffer
*	Summary:	destructor
-------------------------------------------------------- */
RingBuffer::~RingBuffer()
{
}

/* --------------------------------------------------------
*	Method:		RingBuffer::GetDataSize
*	Summary:	return size of data stored in ringBuffer
-------------------------------------------------------- */
int RingBuffer::GetDataSize()
{
	return m_size;
	/*
	if (m_headPos > m_tailPos)
	{
		return m_headPos - m_tailPos;
	}
	else
	{
		return m_capacity - m_tailPos + m_headPos;
	}
	*/
}

/* --------------------------------------------------------
*	Method:		RingBuffer::GetFreeSize
*	Summary:	return size of data that can be stored
-------------------------------------------------------- */
int RingBuffer::GetFreeSize()
{
	return m_capacity - m_size;
	/*
	if (m_headPos > m_tailPos)
	{
		return m_capacity - m_headPos + m_tailPos;
	}
	else
	{
		return m_tailPos - m_headPos;
	}
	*/
}

/* --------------------------------------------------------
*	Method:		RingBuffer::WriteBuffer
*	Summary:	write data in ring buffer
*	Args:		char* data
*					data stored in buffer
*				int writeSize
*					write data size
-------------------------------------------------------- */
bool RingBuffer::WriteBuffer(char* data, int writeSize)
{
	// valid check
	if (GetFreeSize() < writeSize)
	{
		return false;
	}

	// write data
	int needToWriteSize = writeSize;
	int currentWritePos = 0;
	int chunkWriteSize = 0;

	while (needToWriteSize > 0)
	{
		chunkWriteSize = GetChunkWriteSize();

		if (chunkWriteSize < needToWriteSize)
		{
			// 현재 적을 공간의 사이즈보다 데이터 사이즈가 큰 경우
			std::memcpy(GetWriteBuf(), &data[currentWritePos], chunkWriteSize);
			
			m_headPos = (m_headPos + chunkWriteSize) % m_capacity;
			
			needToWriteSize -= chunkWriteSize;
			currentWritePos += chunkWriteSize;
		}
		else
		{
			// 현재 적을 공간의 사이즈보다 데이터 사이즈가 작은 경우
			std::memcpy(GetWriteBuf(), &data[currentWritePos], needToWriteSize);
			
			m_headPos = (m_headPos + needToWriteSize) % m_capacity;

			break;
		}
	}
	m_size += writeSize;

	return true;
}

/* --------------------------------------------------------
*	Method:		RingBuffer::ReadBuffer
*	Summary:	read data from ring buffer
*	Args:		char* destData
*					Buffer to get read data from
*				int readSize
*					read data size
-------------------------------------------------------- */
bool RingBuffer::ReadBuffer(char* destData, int readSize)
{
	// valid check
	if (GetDataSize() < readSize)
	{
		return false;
	}

	// read data
	int chunkReadSize = 0;
	int currentReadPos = 0;
	int needToReadSize = readSize;

	while (needToReadSize > 0)
	{
		chunkReadSize = GetChunkReadSize();

		if (chunkReadSize < needToReadSize)
		{
			std::memcpy(&destData[currentReadPos], GetReadBuf(), chunkReadSize);

			m_tailPos = (m_tailPos + chunkReadSize) % m_capacity;

			currentReadPos += chunkReadSize;
			needToReadSize -= chunkReadSize;
		}
		else
		{
			std::memcpy(&destData[currentReadPos], GetReadBuf(), needToReadSize);

			m_tailPos = (m_tailPos + needToReadSize) % m_capacity;

			break;
		}
	}
	m_size -= readSize;

	return true;
}

/* --------------------------------------------------------
*	Method:		RingBuffer::OnWriteBuffer
*	Summary:	Apply write data size in ringBuffer
*	Args:		int writeSize
*					write data size
-------------------------------------------------------- */
bool RingBuffer::OnWriteBuffer(int writeSize)
{
	if (GetFreeSize() < writeSize)
	{
		return false;
	}

	m_headPos = (m_headPos + writeSize) % m_capacity;
	m_size += writeSize;

	return true;
}

/* --------------------------------------------------------
*	Method:		RingBuffer::OnReadBuffer
*	Summary:	Apply read data size in ringBuffer
*	Args:		int readSize
*					read data size
-------------------------------------------------------- */
bool RingBuffer::OnReadBuffer(int readSize)
{
	if (GetDataSize() < readSize)
	{
		return false;
	}

	m_tailPos = (m_tailPos + readSize) % m_capacity;
	m_size -= readSize;

	return true;
}

/* --------------------------------------------------------
*	Method:		RingBuffer::GetChunkWriteSize
*	Summary:	Returns the size of data 
				that can be write at one time.
-------------------------------------------------------- */
int RingBuffer::GetChunkWriteSize()
{
	if (m_headPos >= m_tailPos)
	{
		return m_capacity - m_headPos;
	}
	else
	{
		return m_tailPos - m_headPos;
	}
}

/* --------------------------------------------------------
*	Method:		RingBuffer::GetChunkReadSize
*	Summary:	Returns the size of data
				that can be read at one time.
-------------------------------------------------------- */
int RingBuffer::GetChunkReadSize()
{
	if (m_headPos >= m_tailPos)
	{
		return m_headPos - m_tailPos;
	}
	else
	{
		return m_capacity - m_tailPos;
	}
}

/* --------------------------------------------------------
*	Method:		RingBuffer::AdjustPos
*	Summary:	adjust position of head and tail.
*				Used when the remaining space is insufficient
*				when using the buffer for reading data.
-------------------------------------------------------- */
void RingBuffer::AdjustPos()
{
	if (m_headPos >= m_tailPos)
	{
		int dataSize = GetDataSize();
		if (m_headPos == m_tailPos)
		{
			m_headPos = 0;
			m_tailPos = 0;
		}
		else if(GetChunkWriteSize() < m_capacity / 3)
		{
			::memcpy(&m_buffer[0], &m_buffer[m_tailPos], dataSize);
			m_tailPos = 0;
			m_headPos = dataSize;
		}
	}
	else
	{
		int rDataSize = m_capacity - m_tailPos;
		int lDataSize = m_headPos;

		::memcpy(&m_buffer[rDataSize], &m_buffer[0], lDataSize);
		::memcpy(&m_buffer[0], &m_buffer[m_tailPos], rDataSize);

		m_tailPos = 0;
		m_headPos = rDataSize + lDataSize;
	}
}

