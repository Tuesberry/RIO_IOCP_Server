#pragma once

#include "Common.h"
//#include "MemoryPool.h"

/* --------------------------------------------------------
*	class:		ObjectPool
*	Summary:	Object Pool using SList
-------------------------------------------------------- */
template<typename TObject>
class ObjectPool
{
public:
	ObjectPool();	
	ObjectPool(const ObjectPool& other) = delete;
	ObjectPool(ObjectPool&& other) = delete;
	ObjectPool& operator=(const ObjectPool& other) = delete;
	ObjectPool& operator=(ObjectPool&& other) = delete;
	~ObjectPool() = default;

	void Push(TObject* ptr);
	TObject* Pop();

private:
	SLIST_HEADER m_header;

	atomic<int> m_useCount;
	atomic<int> m_allocCount;
};

template<typename TObject>
inline ObjectPool<TObject>::ObjectPool()
	: m_header()
	, m_useCount(0)
	, m_allocCount(0)
{
}

template<typename TObject>
inline void ObjectPool<TObject>::Push(TObject* ptr)
{
	::InterlockedPushEntrySList(&m_header, static_cast<PSLIST_ENTRY>(ptr));

	m_useCount.fetch_sub(1);
}

template<typename TObject>
inline TObject* ObjectPool<TObject>::Pop()
{
	TObject* object = static_cast<TObject*>(::InterlockedPopEntrySList(&m_header));

	if (object == nullptr)
	{
		object = reinterpret_cast<TObject*>(::_aligned_malloc(sizeof(TObject), 16));
		m_allocCount.fetch_add(1);
	}

	m_useCount.fetch_add(1);

	return object;
}

/* --------------------------------------------------------
*	class:		Poolable
*	Summary:	Objects that can be included in a ObjectPool
-------------------------------------------------------- */
template<typename TObject>
class Poolable : public SLIST_ENTRY
{
public:
	template<typename... Args>
	static void* operator new(size_t objSize, Args&&... args)
	{
		TObject* object = s_pool.Pop();

		if (object == nullptr)
		{
			HandleError("Poolable::new");
			return nullptr;
		}
		
		return static_cast<void*>(object);
	}

	static void operator delete(void* obj)
	{
		s_pool.Push(static_cast<TObject*>(obj));
	}

private:
	static ObjectPool<TObject> s_pool;
};

template<typename TObject>
ObjectPool<TObject> Poolable<TObject>::s_pool{};


