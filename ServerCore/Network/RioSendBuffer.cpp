#include "RioSendBuffer.h"

/* --------------------------------------------------------
*	Method:		RioSendBuffer::RioSendBuffer
*	Summary:	constructor
-------------------------------------------------------- */
RioSendBuffer::RioSendBuffer(int bufferSize)
	: RingBuffer(nullptr, bufferSize)
	, m_sendPos(0)
{
	AllocateBuffer();
}

/* --------------------------------------------------------
*	Method:		RioSendBuffer::~RioSendBuffer
*	Summary:	destructor
-------------------------------------------------------- */
RioSendBuffer::~RioSendBuffer()
{
	// free
	VirtualFreeEx(GetCurrentProcess(), m_buffer, 0, MEM_RELEASE);
}

/* --------------------------------------------------------
*	Method:		RioSendBuffer::GetChunkSendSize
*	Summary:	Returns the size of data
*				that can be send at one time.
-------------------------------------------------------- */
int RioSendBuffer::GetChunkSendSize()
{
	if (m_headPos >= m_sendPos)
	{
		return m_headPos - m_sendPos;
	}
	else
	{
		return m_capacity - m_sendPos;
	}
}

/* --------------------------------------------------------
*	Method:		RioSendBuffer::GetSendDataSize
*	Summary:	Returns the size of data should be sent
-------------------------------------------------------- */
int RioSendBuffer::GetSendDataSize()
{
	if (m_headPos >= m_sendPos)
	{
		return m_headPos - m_sendPos;
	}
	else
	{
		return m_capacity - m_sendPos + m_headPos;
	}
}

/* --------------------------------------------------------
*	Method:		RioSendBuffer::OnSendBuffer
*	Summary:	Apply send data size 
*	Args:		int sendSize
*					send data size
-------------------------------------------------------- */
bool RioSendBuffer::OnSendBuffer(int sendSize)
{
	if (GetSendDataSize() < sendSize)
		return false;

	m_sendPos = (m_sendPos + sendSize) % m_capacity;

	return true;
}

/* --------------------------------------------------------
*	Method:		RioSendBuffer::AllocateBuffer
*	Summary:	virtual allocate buffer
-------------------------------------------------------- */
void RioSendBuffer::AllocateBuffer()
{
	// alloc
	m_buffer = reinterpret_cast<char*>(VirtualAllocEx(GetCurrentProcess(), 0, m_capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	if (m_buffer == nullptr)
	{
		HandleError("AllocateBuffer");
		return;
	}
}
