#include "NetBuffer.h"

/* --------------------------------------------------------
*	Method:		NetBuffer::NetBuffer
*	Summary:	constructor
*	Args:		int bufferSize
*					buffer data size
-------------------------------------------------------- */
NetBuffer::NetBuffer(int bufferSize)
	: RingBuffer(nullptr, bufferSize)
	, m_bufferVec()
{
	m_bufferVec.reserve(bufferSize);
	m_bufferVec.resize(bufferSize);
	m_buffer = &m_bufferVec[0];
}

/* --------------------------------------------------------
*	Method:		NetBuffer::~NetBuffer
*	Summary:	destructor
-------------------------------------------------------- */
NetBuffer::~NetBuffer()
{

}
