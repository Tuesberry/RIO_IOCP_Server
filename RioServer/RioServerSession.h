#pragma once

#include "Common.h"

#include "Network/RioSession.h"

class RioServerSession : public RioSession
{
public:
	RioServerSession();
	RioServerSession(const RioServerSession& other) = delete;
	RioServerSession(RioServerSession&& other) = delete;
	RioServerSession& operator=(const RioServerSession& other) = delete;
	RioServerSession& operator=(RioServerSession&& other) = delete;
	~RioServerSession() = default;

public:
	virtual void OnConnected() override;
	virtual void OnRecvPacket(char* buffer, int len) override;
	virtual void OnSend(int len) override;
	virtual void OnDisconnected() override;
};