#pragma once

#include "Common.h"

#include "SockAddress.h"
//#include "RecvBuffer.h"
#include "SendBuffer.h"
#include "RioEvent.h"

#include "RioSendBuffer.h"

class RioBuffer;
class RioCore;

/* --------------------------------------------------------
*	class:		RioSession
*	Summary:	registered io session
-------------------------------------------------------- */
class RioSession : public enable_shared_from_this<RioSession>
{
	enum
	{
		BUFFER_SIZE = 65536,
		MAX_RECV_RQ_SIZE = 32,
		MAX_SEND_RQ_SIZE = 64,
		RECV_BUFF_COUNT = 1,
		SEND_BUFF_COUNT = 1,
	};

public:
	RioSession();
	RioSession(const RioSession& other) = delete;
	RioSession(RioSession&& other) = delete;
	RioSession& operator=(const RioSession& other) = delete;
	RioSession& operator=(RioSession&& other) = delete;
	~RioSession() = default;

	// get
	SOCKET& GetSocket() { return m_socket; }
	SockAddress GetAddr() { return m_address; }
	
	// set
	void SetAddr(SOCKADDR_IN addr) { m_address = SockAddress(addr); }
	void SetAddr(SockAddress sockAddr) { m_address = sockAddr; }
	void SetCore(shared_ptr<RioCore> core) { m_rioCore = core; }

	// state check
	bool IsConnected() { return m_bConnected; }
	bool IsEmptySendQueue() { return m_sendBufQueue.empty(); }
	bool IsAllocated() { return m_bAllocated; }

	// networking
	void Disconnect();
	void Send(shared_ptr<SendBuffer> sendBuffer);

	// dispatch
	void Dispatch(RioEvent* rioEvent, int bytesTransferred = 0);

	// DeferredSend & Commit
	bool SendDeferred();
	void SendCommit();

public:
	void RegisterRecv();
	bool RegisterSend(int dataLength, int dataOffset);

	void ProcessConnect();
	void ProcessRecv(int bytesTransferred);
	void ProcessSend(int bytesTransferred, RioSendEvent* sendEvent);

public:
	virtual void OnConnected() {}
	virtual int OnRecv(char* buffer, int len) final;
	virtual void OnRecvPacket(char* buffer, int len) abstract;
	virtual void OnSend(int len) {}
	virtual void OnDisconnected() {}

private:
	bool InitSession();
	bool AllocBuffer();
	bool CreateRequestQueue();

private:
	// socket
	SOCKET m_socket;
	SockAddress m_address;
	atomic<bool> m_bConnected;
	atomic<bool> m_bAllocated;

	// rio core
	weak_ptr<RioCore> m_rioCore;

	// request queue
	RIO_RQ m_requestQueue;

	// RioEvent
	RioRecvEvent m_recvEvent;
	RioSendEvent m_sendEvent;

	// Rio Send
	queue<shared_ptr<SendBuffer>> m_sendBufQueue;
	mutex m_sendQueueLock;
	atomic<bool> m_bSendRegistered;

	// send deferred
	atomic<int> m_sendCnt;
	atomic<int> m_commintCnt;
	int m_sendStartTime;
	
	// rio Buffer
	RIO_BUFFERID m_recvBufId;
	RIO_BUFFERID m_sendBufId;

	// Data Buffer
	shared_ptr<RioBuffer> m_recvBuffer;

	mutex m_sendBufferLock;
	shared_ptr<RioSendBuffer> m_sendBuffer;

public:

};