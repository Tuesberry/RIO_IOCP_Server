#pragma once

#include "Common.h"

/* --------------------------------------------------------
*	class:		RWLock
*	Summary:	Read Write Lock
-------------------------------------------------------- */
class RWLock final
{
	enum : unsigned __int32
	{
		WRITE_LOCK_MASK = 0xFFFF0000,
		READ_LOCK_MASK = 0x0000FFFF,
		MAX_SPIN_COUNT = 5000,
		EMPTY_FLAG = 0x00000000
	};

public:
	RWLock();
	RWLock(const RWLock& other) = delete;
	RWLock(RWLock&& other) = delete;
	RWLock& operator=(const RWLock& other) = delete;
	RWLock& operator=(RWLock&& other) = delete;
	~RWLock() = default;

	void ReadLock();
	void ReadUnlock();

	void WriteLock();
	void WriteUnlock();

private:
	atomic<unsigned __int32> m_lockFlag;
	int m_writeCnt = 0;
};

/* --------------------------------------------------------
*	class:		ReadLockGuard
*	Summary:	Read lock guard
-------------------------------------------------------- */

class ReadLockGuard final
{
public:
	ReadLockGuard(RWLock& readLock) :m_readLock(readLock) { m_readLock.ReadLock(); }
	~ReadLockGuard() { m_readLock.ReadUnlock(); }
	
	ReadLockGuard() = delete;
	ReadLockGuard(const ReadLockGuard& other) = delete;
	ReadLockGuard(ReadLockGuard&& other) = delete;
	ReadLockGuard& operator=(const ReadLockGuard& other) = delete;
	ReadLockGuard& operator=(ReadLockGuard&& other) = delete;

private:
	RWLock& m_readLock;
};

/* --------------------------------------------------------
*	class:		WriteLockGuard
*	Summary:	Write lock guard
-------------------------------------------------------- */

class WriteLockGuard final
{
public:
	WriteLockGuard(RWLock& writeLock) :m_writeLock(writeLock) { m_writeLock.WriteLock(); }
	~WriteLockGuard() { m_writeLock.WriteUnlock(); }

	WriteLockGuard() = delete;
	WriteLockGuard(const WriteLockGuard& other) = delete;
	WriteLockGuard(WriteLockGuard&& other) = delete;
	WriteLockGuard& operator=(const WriteLockGuard& other) = delete;
	WriteLockGuard& operator=(WriteLockGuard&& other) = delete;

private:
	RWLock& m_writeLock;
};