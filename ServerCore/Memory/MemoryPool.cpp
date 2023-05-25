#include "MemoryPool.h"

MemoryPool::MemoryPool(int size)
	: m_allocSize(size)
	, m_header()
	, m_useCount(0)
	, m_allocCount(0)
{
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	ptr->allocSize = 0;

	::InterlockedPushEntrySList(&m_header, static_cast<PSLIST_ENTRY>(ptr));

	m_useCount.fetch_sub(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&m_header));

	if (memory == nullptr)
	{
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(m_allocSize, SLIST_ALIGNMENT));
		m_allocCount.fetch_add(1);
	}
	else
	{
		if (memory->allocSize == 0)
		{
			HandleError("MemoryPool::Pop");
			return nullptr;
		}
	}

	m_useCount.fetch_add(1);

	return memory;
}
