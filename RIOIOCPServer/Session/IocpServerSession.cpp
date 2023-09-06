#include "IOCPServerSession.h"

#include "PacketHandler.h"
#include "../Protocol/Protocol.pb.h"

#include "../Contents/Room.h"
#include "../Contents/Player.h"

#ifdef IOCP
/* --------------------------------------------------------
*	Method:		IocpServerSession::IocpServerSession
*	Summary:	Constructor
-------------------------------------------------------- */
IocpServerSession::IocpServerSession()
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
*	Method:		IocpServerSession::~IocpServerSession
*	Summary:	Destructor
-------------------------------------------------------- */
IocpServerSession::~IocpServerSession()
{
    if (m_ownPlayer != nullptr)
    {
        if (m_ownPlayer->m_playerState == State::Connected)
        {
            m_ownPlayer->m_playerState = State::Disconnected;
            gRoom->DoAsync(&Room::Logout, m_ownPlayer);
        }
    }
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::OnRecvPacket
*	Summary:	called when receive packet
*   Args:       char* buffer
*                   buffer containing received data
*               int len
*                   data length
-------------------------------------------------------- */
void IocpServerSession::OnRecvPacket(char* buffer, int len)
{
    bool result = PacketHandler<IocpServerSession>::HandlePacket(
        static_pointer_cast<IocpServerSession>(shared_from_this()), buffer, len);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::OnSend
*	Summary:	called after sending packet
*   Args:       int len
*                   sending data length
-------------------------------------------------------- */
void IocpServerSession::OnSend(int len)
{
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::OnDisconnected
*	Summary:	called after disconnecting from client
-------------------------------------------------------- */
void IocpServerSession::OnDisconnected()
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
*	Method:		IocpServerSession::SendMoveMsg
*	Summary:	send move message to client
*   Args:       int targetId
*                   target client id
*               PlayerInfo pInfo
*                   target player's position info
-------------------------------------------------------- */
void IocpServerSession::SendMoveMsg(int targetId, PlayerInfo& pInfo)
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

    shared_ptr<NetBuffer> NetBuffer = BufferHandler::CreateNetBuffer(pkt);
    Send(NetBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendEnterMsg
*	Summary:	send enter message to client
*   Args:       int targetId
*                   target client id
*               PlayerInfo pInfo
*                   target player's position info
-------------------------------------------------------- */
void IocpServerSession::SendEnterMsg(int targetId, PlayerInfo& pInfo)
{
    Protocol::S2C_ENTER pkt;
    pkt.set_session_id(m_sessionId);
    pkt.set_target_id(targetId);
    pkt.set_time_stamp(pInfo.timeStamp);
    pkt.set_player_type(pInfo.playerType);
    pkt.set_target_str_id(pInfo.playerId);

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

    shared_ptr<NetBuffer> NetBuffer = BufferHandler::CreateNetBuffer(pkt);
    Send(NetBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendLeaveMsg
*	Summary:	send leave message to client
*   Args:       int targetId
*                   target client id
-------------------------------------------------------- */
void IocpServerSession::SendLeaveMsg(int targetId)
{
    Protocol::S2C_LEAVE pkt;
    pkt.set_session_id(m_sessionId);
    pkt.set_target_id(targetId);

    shared_ptr<NetBuffer> NetBuffer = BufferHandler::CreateNetBuffer(pkt);
    Send(NetBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendLeaveMsg
*	Summary:	send login result message to client
*   Args:       bool result
*                   login result
*               int player_type
*                   player's type
-------------------------------------------------------- */
void IocpServerSession::SendLoginResultMsg(bool result, int player_type)
{
    Protocol::S2C_LOGIN_RESULT pkt;
    pkt.set_session_id(m_sessionId);
    pkt.set_result(result);
    pkt.set_login_time(m_loginTime);
    pkt.set_player_type(player_type);
    pkt.set_player_str_id(m_playerId);

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

    shared_ptr<NetBuffer> NetBuffer = BufferHandler::CreateNetBuffer(pkt);

    Send(NetBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendMoveResultMsg
*	Summary:	send moveSync result message to client
-------------------------------------------------------- */
void IocpServerSession::SendMoveResultMsg()
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

    shared_ptr<NetBuffer> NetBuffer = BufferHandler::CreateNetBuffer(pkt);
    Send(NetBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendChat
*	Summary:	send chat to client
*   Args:       int targetId
*                   targetId who owns the chat
*               string& playerId
*                   player id
*               string& chat
*                   chatting to send
-------------------------------------------------------- */
void IocpServerSession::SendChat(int targetId, string& playerStrId, string& chat)
{
    Protocol::S2C_CHAT pkt;

    pkt.set_session_id(m_sessionId);
    pkt.set_target_id(targetId);
    pkt.set_target_str_id(playerStrId);
    pkt.set_chat(chat);

    shared_ptr<NetBuffer> NetBuffer = BufferHandler::CreateNetBuffer(pkt);
    Send(NetBuffer);
}
#endif // IOCP