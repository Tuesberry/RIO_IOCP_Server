#include "RioBuffer.h"

RioBuffer::RioBuffer(int bufferSize)
	: m_buffer(nullptr)
	, m_bufSize(bufferSize)
{
	AllocateBuffer();
}

RioBuffer::~RioBuffer()
{
	// free
	VirtualFreeEx(GetCurrentProcess(), m_buffer, 0, MEM_RELEASE);
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
