#include "BufferHelper.h"

BufferWriter::BufferWriter(BYTE* buffer, int size, int pos)
	: m_buffer(buffer)
	, m_size(size)
	, m_pos(pos)
{
}

bool BufferWriter::Write(void* src, int len)
{
	if (FreeSize() < len)
		return false;

	::memcpy(&m_buffer[m_pos], src, len/*바이트 수*/);
	m_pos += len;
	return true;
}

int BufferWriter::FreeSize()
{
	return m_size - m_pos;
}

BufferReader::BufferReader(BYTE* buffer, int size, int pos)
	: m_buffer(buffer)
	, m_size(size)
	, m_pos(pos)
{
}

bool BufferReader::Read(void* dest, int len)
{
	if (FreeSize() < len)
		return false;

	::memcpy(dest, &m_buffer[m_pos], len);
	m_pos += len;
	return true;
}
