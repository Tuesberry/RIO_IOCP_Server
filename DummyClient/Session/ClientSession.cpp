#include "ClientSession.h"
#include "PacketHandler.h"
#include "Network/IocpService.h"
#include "Network/IocpClient.h"
#include "../StressTest/TestSessionManager.h"

atomic<int> gSessionID = 0;

/* --------------------------------------------------------
*	Method:		ClientSession::ClientSession
*	Summary:	Constructor
-------------------------------------------------------- */
ClientSession::ClientSession()
	: m_sessionID(0)
	, m_moveComp()
	, m_bLogin(false)
	, m_bStartLogin(false)
	, m_bConnect(false)
{
}

/* --------------------------------------------------------
*	Method:		ClientSession::ClientSession
*	Summary:	Destructor
-------------------------------------------------------- */
ClientSession::~ClientSession()
{
}

/* --------------------------------------------------------
*	Method:		ClientSession::OnConnected
*	Summary:	execute after connecting to server
-------------------------------------------------------- */
void ClientSession::OnConnected()
{
	// set sessionID
	m_sessionID = gSessionID.fetch_add(1);
	m_sessionID++;

	// connect OK
	m_bConnect = true;

	// stress test
	gTestSessionMgr.AddSession(m_sessionID, static_pointer_cast<ClientSession>(shared_from_this()));
}

/* --------------------------------------------------------
*	Method:		ClientSession::OnRecvPacket
*	Summary:	execute after recv packet
*	Args:		char* buffer
*					recv data buffer
*				int len
*					recv data length
-------------------------------------------------------- */
void ClientSession::OnRecvPacket(char* buffer, int len)
{
	bool result = PacketHandler::HandlePacket(
		static_pointer_cast<ClientSession>(shared_from_this()), buffer, len);

	if (result == false)
	{
		HandleError("ClientSession::OnRecvPacket");
		Disconnect();
	}
}

/* --------------------------------------------------------
*	Method:		ClientSession::OnSend
*	Summary:	execute after sending to server
-------------------------------------------------------- */
void ClientSession::OnSend(int len)
{
}

/* --------------------------------------------------------
*	Method:		ClientSession::OnDisconnected
*	Summary:	execute after disconnecting to server
-------------------------------------------------------- */
void ClientSession::OnDisconnected()
{
	cout << "OnDisconnected" << endl;
}

/* --------------------------------------------------------
*	Method:		ClientSession::SendLogin
*	Summary:	send login packet to server
-------------------------------------------------------- */
void ClientSession::SendLogin()
{
	m_bStartLogin = true;

	Protocol::C2S_LOGIN pkt;

	pkt.set_player_id("test" + to_string(gTestSessionMgr.m_loginNum.fetch_add(1)));
	pkt.set_player_pw("qwer1234");

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(1, 5);
	pkt.set_player_type(dist(gen));

	pkt.set_login_time(duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count());
	
	shared_ptr<NetBuffer> NetBuffer = PacketHandler::CreateNetBuffer(pkt);
	Send(NetBuffer);
}

/* --------------------------------------------------------
*	Method:		ClientSession::SendMove
*	Summary:	send Move packet to server
-------------------------------------------------------- */
void ClientSession::SendMove()
{
	// update move
	m_moveComp.Update(1);

	Protocol::C2S_MOVE pkt;
	pkt.set_session_id(m_sessionID);
	pkt.set_request_result(true);
	
	Protocol::PLAYER_POS_INFO* posInfo = pkt.mutable_pos_info();
	PositionInfo& pos = m_moveComp.m_positionInfo;
	posInfo->set_x(pos.x);
	posInfo->set_y(pos.y);
	posInfo->set_z(pos.z);
	posInfo->set_yaw(pos.yaw);
	posInfo->set_pitch(pos.pitch);
	posInfo->set_roll(pos.roll);
	posInfo->set_vx(pos.vx);
	posInfo->set_vy(pos.vy);
	posInfo->set_vz(pos.vz);
	posInfo->set_ax(pos.ax);
	posInfo->set_ay(pos.ay);
	posInfo->set_az(pos.az);

	pkt.set_time_stamp(duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count());

	shared_ptr<NetBuffer> NetBuffer = PacketHandler::CreateNetBuffer(pkt);
	Send(NetBuffer);
}

void MoveComponent::Update(float deltaTime)
{
	// update direction & velocity
	UpdateVelocityByNewDirection();

	// update position
	UpdateNewPosition(deltaTime);
}

pair<int, int> MoveComponent::GetNewDirection()
{
	::random_device rd;
	::mt19937 mt(rd());

	::uniform_int_distribution<int> dist(-1, 1);

	int dy = dist(mt);
	int dx = dist(mt);

	if (m_positionInfo.x < -9900)
	{
		dx = 1;
	}
	else if (m_positionInfo.x > 9900)
	{
		dx = -1;
	}
	if (m_positionInfo.y < -9900)
	{
		dy = 1;
	}
	else if (m_positionInfo.y > 9900)
	{
		dy = -1;
	}
	
	return { dy, dx };
}

void MoveComponent::UpdateVelocityByNewDirection()
{
	int vx, vy;
	tie(vy, vx) = GetNewDirection();

	if (vx == 0 || vy == 0)
	{
		m_positionInfo.vx = vx;
		m_positionInfo.vy = vy;
	}
	else
	{
		// normalize
		float sum = sqrtf(powf(vx, 2) + powf(vy, 2));
		m_positionInfo.vx = vx / sum;
		m_positionInfo.vy = vy / sum;
	}
}

void MoveComponent::UpdateNewPosition(float deltaTime)
{
	// x
	m_positionInfo.x += m_positionInfo.vx * m_speed * deltaTime;

	// y
	m_positionInfo.y += m_positionInfo.vy * m_speed * deltaTime;
}
