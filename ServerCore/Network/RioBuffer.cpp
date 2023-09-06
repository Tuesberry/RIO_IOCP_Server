#include "RioBuffer.h"

/* --------------------------------------------------------
*	Method:		RioBuffer::RioBuffer
*	Summary:	constructor
-------------------------------------------------------- */
RioBuffer::RioBuffer(int bufferSize)
	: RingBuffer(nullptr, bufferSize)
	, m_sendPos(0)
{
	AllocateBuffer();
}

/* --------------------------------------------------------
*	Method:		RioBuffer::~RioBuffer
*	Summary:	destructor
-------------------------------------------------------- */
RioBuffer::~RioBuffer()
{
	// free
	VirtualFreeEx(GetCurrentProcess(), m_buffer, 0, MEM_RELEASE);
}

/* --------------------------------------------------------
*	Method:		RioBuffer::GetChunkSendSize
*	Summary:	Returns the size of data
*				that can be send at one time.
-------------------------------------------------------- */
int RioBuffer::GetChunkSendSize()
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
*	Method:		RioBuffer::GetSendDataSize
*	Summary:	Returns the size of data should be sent
-------------------------------------------------------- */
int RioBuffer::GetSendDataSize()
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
*	Method:		RioBuffer::OnSendBuffer
*	Summary:	Apply send data size 
*	Args:		int sendSize
*					send data size
-------------------------------------------------------- */
bool RioBuffer::OnSendBuffer(int sendSize)
{
	if (GetSendDataSize() < sendSize)
		return false;

	m_sendPos = (m_sendPos + sendSize) % m_capacity;

	return true;
}

/* --------------------------------------------------------
*	Method:		RioBuffer::AllocateBuffer
*	Summary:	virtual allocate buffer
-------------------------------------------------------- */
void RioBuffer::AllocateBuffer()
{
	// alloc
	m_buffer = reinterpret_cast<char*>(VirtualAllocEx(GetCurrentProcess(), 0, m_capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	if (m_buffer == nullptr)
	{
		HandleError("AllocateBuffer");
		return;
	}
}
