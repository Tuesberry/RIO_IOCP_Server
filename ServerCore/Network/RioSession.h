#pragma once

#include "Common.h"

#include "SockAddress.h"
//#include "RecvBuffer.h"
#include "SendBuffer.h"
#include "RioEvent.h"

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
		MAX_SEND_RQ_SIZE = 32,
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
	
	// networking
	bool Connect();
	void Disconnect();
	void Send(shared_ptr<SendBuffer> sendBuffer);

	// dispatch
	void Dispatch(RioEvent* rioEvent, int bytesTransferred = 0);

	// DeferredSend & Commit
	bool SendDeferred();
	void SendCommit();

public:
	void RegisterRecv();
	void RegisterSend(int dataLength, int dataOffset);

	void ProcessConnect();
	void ProcessRecv(int bytesTransferred);
	void ProcessSend(int bytesTransferred);

public:
	virtual void OnConnected() {}
	//virtual int OnRecv(char* buffer, int len) abstract;
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

	// rio core
	weak_ptr<RioCore> m_rioCore;

	// request queue
	RIO_RQ m_requestQueue;

	// RioReceive
	RioRecvEvent m_recvEvent;

	// Rio Send
	queue<shared_ptr<SendBuffer>> m_sendBufQueue;
	mutex m_sendQueueLock;
	RioSendEvent m_sendEvent;
	atomic<bool> m_bSendRegistered;
	
	// rio Buffer
	RIO_BUFFERID m_recvBufId;
	RIO_BUFFERID m_sendBufId;

	shared_ptr<RioBuffer> m_recvBuffer;
	shared_ptr<RioBuffer> m_sendBuffer;

	mutex m_lock;
};