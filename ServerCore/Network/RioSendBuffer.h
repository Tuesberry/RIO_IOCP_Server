#pragma once

#include "Common.h"

#include "RingBuffer.h"

/* --------------------------------------------------------
*	class:		RioSendBuffer
*	Summary:	ring buffer used for RIO
-------------------------------------------------------- */
class RioSendBuffer : public RingBuffer
{
public:
	RioSendBuffer(int bufferSize);

	RioSendBuffer() = delete;
	RioSendBuffer(const RioSendBuffer& other) = delete;
	RioSendBuffer(RioSendBuffer&& other) = delete;
	RioSendBuffer& operator=(const RioSendBuffer& other) = delete;
	RioSendBuffer& operator=(RioSendBuffer&& other) = delete;
	~RioSendBuffer();

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