#pragma once
#include "ClientSession.h"
#include "ClientPacketHandler.h"
#include "Utils/ConsoleOutputManager.h"
#include "Utils/BufferHelper.h"
#include "Network/IocpService.h"
#include "Network/IocpClient.h"

atomic<int> gSessionID = 0;

ClientSession::ClientSession()
	: m_sessionID(0)
	, m_posX(0)
	, m_posY(0)
	, m_moveTime(0)
	, m_bAddDelay(false)
	, m_bLogin(false)
	, m_bStartLogin(false)
	, m_bConnect(false)
{
}

ClientSession::~ClientSession()
{
	//cout << "~ClientSession" << endl;
}

void ClientSession::OnConnected()
{
	// set sessionID
	m_sessionID = gSessionID.fetch_add(1);
	m_sessionID++;

	// connect OK
	m_bConnect = true;
}

void ClientSession::OnRecvPacket(BYTE* buffer, int len)
{
	bool result = ClientPacketHandler::HandlePacket(
		static_pointer_cast<ClientSession>(shared_from_this()), buffer, len);

	if (result == false)
		Disconnect();
}

void ClientSession::OnSend(int len)
{
	//cout << "OnSend Len = " << len << endl;
}

void ClientSession::OnDisconnected()
{
	//cout << "Disconnected | session = " << m_sessionID << endl;
}

void ClientSession::SendLogin()
{
	m_bStartLogin = true;

	shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_C2S_LOGIN));
	BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

	PKT_C2S_LOGIN pktLogin;
	pktLogin.header.id = PROTO_ID::LOGIN;
	pktLogin.header.size = sizeof(PKT_C2S_LOGIN);
	pktLogin.id = m_sessionID;
	pktLogin.loginTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

	bw.Write(&pktLogin, sizeof(PKT_C2S_LOGIN));

	sendBuffer->OnWrite(sizeof(PKT_C2S_LOGIN));
	Send(sendBuffer);
}

void ClientSession::SendMove()
{
	shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_C2S_MOVE));
	BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

	PKT_C2S_MOVE pktMove;
	pktMove.header.id = PROTO_ID::C2S_MOVE;
	pktMove.header.size = sizeof(PKT_C2S_MOVE);
	pktMove.id = m_sessionID;
	pktMove.direction = rand() % 4;
	pktMove.moveTime = static_cast<unsigned>(duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count());

	bw.Write(&pktMove, sizeof(PKT_C2S_MOVE));

	sendBuffer->OnWrite(sizeof(PKT_C2S_MOVE));
	Send(sendBuffer);
}
