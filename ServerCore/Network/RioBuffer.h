#pragma once

#include "Common.h"

#include "RingBuffer.h"

/* --------------------------------------------------------
*	class:		RioBuffer
*	Summary:	ring buffer used for RIO
-------------------------------------------------------- */
class RioBuffer : public RingBuffer
{
public:
	RioBuffer(int bufferSize);

	RioBuffer() = delete;
	RioBuffer(const RioBuffer& other) = delete;
	RioBuffer(RioBuffer&& other) = delete;
	RioBuffer& operator=(const RioBuffer& other) = delete;
	RioBuffer& operator=(RioBuffer&& other) = delete;
	~RioBuffer();

	// send data
	int GetChunkSendSize();
	int GetSendDataSize();
	int GetSendOffset() { return m_sendPos; }

	bool OnSendBuffer(int sendSize);

private:
	void AllocateBuffer();

	// �����͸� ������ �����ߴ��� üũ�ϱ� ����
	
	// ----tail===send===head---- //
	// ====head----tail====send== //
	
	int m_sendPos;
};