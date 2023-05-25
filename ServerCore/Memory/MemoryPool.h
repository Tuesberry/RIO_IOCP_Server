#pragma once

#include "Common.h"

#define SLIST_ALIGNMENT 16

/* --------------------------------------------------------
*	class:		MemoryHeader
*	Summary:	Memory Header containing SLIST_ENTRY
-------------------------------------------------------- */
DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
	MemoryHeader(int size):allocSize(size){}

	static void* AttachHeader(MemoryHeader* header, int size)
	{
		new(header)MemoryHeader(size); //placement new
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	int allocSize;
};

/* --------------------------------------------------------
*	class:		Memory Pool
*	Summary:	memory pool using SList
-------------------------------------------------------- */
DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int size);

	MemoryPool() = delete;
	MemoryPool(const MemoryPool& other) = delete;
	MemoryPool(MemoryPool&& other) = delete;
	MemoryPool& operator=(const MemoryPool& other) = delete;
	MemoryPool& operator=(MemoryPool&& other) = delete;
	~MemoryPool() = default;

	void Push(MemoryHeader* ptr);
	MemoryHeader* Pop();

private:
	SLIST_HEADER m_header;
	
	int m_allocSize;

	atomic<int> m_useCount;
	atomic<int> m_allocCount;
};