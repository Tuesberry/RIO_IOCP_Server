#include "RioServerSession.h"

#include "Utils/BufferHelper.h"
#include "ServerPacketHandler.h"
#include "../Protocol/Protocol.pb.h"

#include "../Contents/Room.h"
#include "../Contents/Player.h"

/* --------------------------------------------------------
*	Method:		RioServerSession::RioServerSession
*	Summary:	Constructor
-------------------------------------------------------- */
RioServerSession::RioServerSession()
    : m_playerId()
    , m_sessionId(0)
    , m_moveTime(0)
    , m_loginTime(0)
    , m_serverProcessTime(0)
    , m_sendTime(0)
    , m_ownPlayer(nullptr)
    , m_bRequestResult(false)
{
}

/* --------------------------------------------------------
*	Method:		RioServerSession::~RioServerSession
*	Summary:	Destructor
-------------------------------------------------------- */
RioServerSession::~RioServerSession()
{
    if (m_ownPlayer == nullptr)
    {
        return;
    }

    if (m_ownPlayer->m_playerState == State::Connected)
    {
        m_ownPlayer->m_playerState = State::Disconnected;
        gRoom->DoAsync(&Room::Logout, m_ownPlayer);
    }
}

/* --------------------------------------------------------
*	Method:		RioServerSession::OnRecvPacket
*	Summary:	called when receive packet
*   Args:       BYTE* buffer
*                   buffer containing received data
*               int len
*                   data length
-------------------------------------------------------- */
void RioServerSession::OnRecvPacket(char* buffer, int len)
{
    bool result = ServerPacketHandler::HandlePacket(
        static_pointer_cast<RioServerSession>(shared_from_this()), (BYTE*)buffer, len);
}

/* --------------------------------------------------------
*	Method:		RioServerSession::OnSend
*	Summary:	called after sending packet
*   Args:       int len
*                   sending data length
-------------------------------------------------------- */
void RioServerSession::OnSend(int len)
{
}

/* --------------------------------------------------------
*	Method:		RioServerSession::OnDisconnected
*	Summary:	called after disconnecting from client
-------------------------------------------------------- */
void RioServerSession::OnDisconnected()
{
    cout << "Disconnect Session : " << m_sessionId << endl;

    if (m_ownPlayer == nullptr)
    {
        return;
    }

    if (m_ownPlayer->m_playerState == State::Connected)
    {
        m_ownPlayer->m_playerState = State::Disconnected;
        gRoom->DoAsync(&Room::Logout, m_ownPlayer);
    }

    m_ownPlayer = nullptr;
}

/* --------------------------------------------------------
*	Method:		RioServerSession::SendMoveMsg
*	Summary:	send move message to client
*   Args:       int targetId
*                   target client id
*               PlayerInfo pInfo
*                   target player's position info
-------------------------------------------------------- */
void RioServerSession::SendMoveMsg(int targetId, PlayerInfo& pInfo)
{
    Protocol::S2C_MOVE pkt;
    pkt.set_session_id(m_sessionId);
    pkt.set_target_id(targetId);
    pkt.set_time_stamp(pInfo.timeStamp);

    Protocol::PLAYER_POS_INFO* pos = pkt.mutable_pos_info();
    pos->set_x(pInfo.x);
    pos->set_y(pInfo.y);
    pos->set_z(pInfo.z);
    pos->set_yaw(pInfo.yaw);
    pos->set_pitch(pInfo.pitch);
    pos->set_roll(pInfo.roll);
    pos->set_vx(pInfo.vx);
    pos->set_vy(pInfo.vy);
    pos->set_vz(pInfo.vz);
    pos->set_ax(pInfo.ax);
    pos->set_ay(pInfo.ay);
    pos->set_az(pInfo.az);

    shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::CreateSendBuffer(pkt);
    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		RioServerSession::SendEnterMsg
*	Summary:	send enter message to client
*   Args:       int targetId
*                   target client id
*               PlayerInfo pInfo
*                   target player's position info
-------------------------------------------------------- */
void RioServerSession::SendEnterMsg(int targetId, PlayerInfo& pInfo)
{
    Protocol::S2C_ENTER pkt;
    pkt.set_session_id(m_sessionId);
    pkt.set_target_id(targetId);
    pkt.set_time_stamp(pInfo.timeStamp);

    Protocol::PLAYER_POS_INFO* pos = pkt.mutable_pos_info();
    pos->set_x(pInfo.x);
    pos->set_y(pInfo.y);
    pos->set_z(pInfo.z);
    pos->set_yaw(pInfo.yaw);
    pos->set_pitch(pInfo.pitch);
    pos->set_roll(pInfo.roll);
    pos->set_vx(pInfo.vx);
    pos->set_vy(pInfo.vy);
    pos->set_vz(pInfo.vz);
    pos->set_ax(pInfo.ax);
    pos->set_ay(pInfo.ay);
    pos->set_az(pInfo.az);

    shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::CreateSendBuffer(pkt);
    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		RioServerSession::SendLeaveMsg
*	Summary:	send leave message to client
*   Args:       int targetId
*                   target client id
-------------------------------------------------------- */
void RioServerSession::SendLeaveMsg(int targetId)
{
    Protocol::S2C_LEAVE pkt;
    pkt.set_session_id(m_sessionId);
    pkt.set_target_id(targetId);

    shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::CreateSendBuffer(pkt);
    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		RioServerSession::SendLeaveMsg
*	Summary:	send login result message to client
*   Args:       bool result
*                   login result
-------------------------------------------------------- */
void RioServerSession::SendLoginResultMsg(bool result)
{
    Protocol::S2C_LOGIN_RESULT pkt;
    pkt.set_session_id(m_sessionId);
    pkt.set_result(result);
    pkt.set_login_time(m_loginTime);

    PlayerInfo& pInfo = m_ownPlayer->m_playerInfo;
    Protocol::PLAYER_POS_INFO* pos = pkt.mutable_pos_info();
    pos->set_x(pInfo.x);
    pos->set_y(pInfo.y);
    pos->set_z(pInfo.z);
    pos->set_yaw(pInfo.yaw);
    pos->set_pitch(pInfo.pitch);
    pos->set_roll(pInfo.roll);
    pos->set_vx(pInfo.vx);
    pos->set_vy(pInfo.vy);
    pos->set_vz(pInfo.vz);
    pos->set_ax(pInfo.ax);
    pos->set_ay(pInfo.ay);
    pos->set_az(pInfo.az);

    shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::CreateSendBuffer(pkt);

    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		RioServerSession::SendMoveResultMsg
*	Summary:	send moveSync result message to client
-------------------------------------------------------- */
void RioServerSession::SendMoveResultMsg()
{
    if (!m_bRequestResult)
    {
        return;
    }

    Protocol::S2C_MOVE_RESULT pkt;

    pkt.set_session_id(m_sessionId);
    pkt.set_time_stamp(m_ownPlayer->m_playerInfo.timeStamp);
    pkt.set_process_time(m_serverProcessTime);
    pkt.set_send_time(m_sendTime);
    pkt.set_recv_time(duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count());

    shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::CreateSendBuffer(pkt);
    Send(sendBuffer);
}