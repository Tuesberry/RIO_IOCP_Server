#pragma once
#include "ClientSession.h"
#include "ClientPacketHandler.h"
#include "Utils/ConsoleOutputManager.h"
#include "Utils/BufferHelper.h"
#include "Network/IocpService.h"

ClientSession::ClientSession()
	: m_sessionID(0)
	, m_posX(10)
	, m_posY(10)
{
}

ClientSession::~ClientSession()
{
	cout << "~ClientSession" << endl;
}

void ClientSession::OnConnected()
{
	// set sessionID
	m_sessionID = GetService()->GetConnectCnt();

	string temp = "Connected To Server |   ID :  " + to_string(m_sessionID);
	GCoutMgr << temp;

	// sendLogin
	SendLogin();
	/*
	int dataLen = sizeof(sendData) / sizeof(BYTE);
	shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(dataLen);
	::memcpy(sendBuffer->GetData(), &sendData, sizeof(sendData));
	sendBuffer->OnWrite(dataLen);
	Send(sendBuffer);
	*/
}

void ClientSession::OnRecvPacket(BYTE* buffer, int len)
{
	bool result = ClientPacketHandler::HandlePacket(
		static_pointer_cast<ClientSession>(shared_from_this()), buffer, len);

	if (result == false)
		Disconnect();

	SendInfo();
}

void ClientSession::OnSend(int len)
{
	//cout << "OnSend Len = " << len << endl;
}

void ClientSession::OnDisconnected()
{
	cout << "Disconnected" << endl;
}

void ClientSession::SendLogin()
{
	shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_LOGIN));
	BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

	PKT_LOGIN pktResult;
	pktResult.header.id = PROTO_ID::LOGIN;
	pktResult.header.size = sizeof(PKT_LOGIN);
	pktResult.id = m_sessionID;

	bw.Write(&pktResult, sizeof(PKT_LOGIN));

	sendBuffer->OnWrite(sizeof(PKT_LOGIN));
	Send(sendBuffer);
}

void ClientSession::SendInfo()
{
	UpdatePos();

	shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_INFO));
	BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

	PKT_INFO pktResult;
	pktResult.header.id = PROTO_ID::INFO;
	pktResult.header.size = sizeof(PKT_INFO);
	pktResult.id = m_sessionID;
	pktResult.posX = m_posX;
	pktResult.poxY = m_posY;

	bw.Write(&pktResult, sizeof(PKT_INFO));

	sendBuffer->OnWrite(sizeof(PKT_INFO));
	Send(sendBuffer);
}

void ClientSession::UpdatePos()
{
	m_posX = rand() % 100;
	m_posY = rand() % 100;
}
