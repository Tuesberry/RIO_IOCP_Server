#pragma once
#include "Common.h"

/* ----------------------------
*		BufferWriter
---------------------------- */

class BufferWriter
{
public:
	BufferWriter(BYTE* buffer, int size, int pos = 0);
	
	BufferWriter() = delete;
	BufferWriter(const BufferWriter& other) = delete;
	BufferWriter(BufferWriter&& other) = delete;
	BufferWriter& operator=(const BufferWriter& other) = delete;
	BufferWriter& operator=(BufferWriter&& other) = delete;
	~BufferWriter() = default;

	bool Write(void* src, int len);
	int FreeSize();

	template<typename T>
	BufferWriter& operator<<(T& src);

	template<typename T>
	BufferWriter& operator<<(const T& src);

private:
	BYTE* m_buffer;
	int m_pos;
	int m_size;
};

template<typename T>
inline BufferWriter& BufferWriter::operator<<(T& src)
{
	int dataLen = sizeof(T);
	if(FreeSize() < dataLen)
		return *this;

	::memcpy(&m_buffer[m_pos], reinterpret_cast<T*>(&src), dataLen);
	m_pos += dataLen;
	return *this;
}

template<typename T>
inline BufferWriter& BufferWriter::operator<<(const T& src)
{
	int dataLen = sizeof(T);
	if (FreeSize() < dataLen)
		return *this;

	::memcpy(&m_buffer[m_pos], reinterpret_cast<T*>(&src), dataLen);
	m_pos += dataLen;
	return *this;
}

/* ----------------------------
*		IocpClient
---------------------------- */

class BufferReader
{
public:
	BufferReader(BYTE* buffer, int size, int pos = 0);

	BufferReader() = delete;
	BufferReader(const BufferReader& other) = delete;
	BufferReader(BufferWriter&& other) = delete;
	BufferReader& operator=(const BufferReader& other) = delete;
	BufferReader& operator=(BufferReader&& other) = delete;
	~BufferReader() = default;

	template<typename T>
	bool Read(T* dest) { return Read(dest, sizeof(T)); }
	bool Read(void* dest, int len);

	int FreeSize() { return m_size - m_pos; }

	template<typename T>
	BufferReader& operator>>(T& dest);

private:
	BYTE* m_buffer;
	int m_pos;
	int m_size;
};

template<typename T>
inline BufferReader& BufferReader::operator>>(T& dest)
{
	int dataLen = sizeof(T);
	if (FreeSize() < dataLen)
		return *this;

	::memcpy(reinterpret_cast<T*>(&dest), &m_buffer[m_pos], dataLen);
	m_pos += dataLen;
	return *this;
}
