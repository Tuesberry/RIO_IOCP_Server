#include "RioServerSession.h"
#include "Utils/BufferHelper.h"
#include "ServerPacketHandler.h"
#include "Room.h"
#include "Player.h"

#if IOCP
#else RIO

/* --------------------------------------------------------
*	Method:		RioServerSession::RioServerSession
*	Summary:	Constructor
-------------------------------------------------------- */
RioServerSession::RioServerSession()
    : m_connectClientId(0)
    , m_moveTime(0)
    , m_loginTime(0)
    , m_serverProcessTime(0)
{
}

/* --------------------------------------------------------
*	Method:		RioServerSession::~RioServerSession
*	Summary:	Destructor
-------------------------------------------------------- */
RioServerSession::~RioServerSession()
{
    cout << m_connectClientId << " | Delete Session " << endl;

    if (m_ownPlayer->m_playerState == State::Connected)
    {
        m_ownPlayer->m_playerState = State::Disconnected;
        //gRoom->DoAsync(&Room::Logout, m_ownPlayer);
        gRoom.Logout(m_ownPlayer);
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
    cout << "Disconnected | session = " << m_connectClientId << endl;
    cout << this->shared_from_this().use_count() << endl;
    cout << m_sendCnt << endl;
    cout << m_sendCompleteCnt << endl;

    if (m_ownPlayer->m_playerState == State::Connected)
    {
        m_ownPlayer->m_playerState = State::Disconnected;
        //gRoom->DoAsync(&Room::Logout, m_ownPlayer);
        gRoom.Logout(m_ownPlayer);
    }
}

/* --------------------------------------------------------
*	Method:		RioServerSession::SendMoveMsg
*	Summary:	send move message to client
*   Args:       int targetId
*                   target client id
*               unsigned short x
*                   position x
*               unsigned short y
*                   position y
-------------------------------------------------------- */
void RioServerSession::SendMoveMsg(int targetId, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_MOVE));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_MOVE pktMove;
    pktMove.header.id = PROTO_ID::S2C_MOVE;
    pktMove.header.size = sizeof(PKT_S2C_MOVE);
    pktMove.id = m_connectClientId;
    pktMove.targetId = targetId;
    pktMove.x = x;
    pktMove.y = y;
    pktMove.moveTime = NULL;
    pktMove.processTime = NULL;

    if (targetId == m_connectClientId)
    {
        // 자기 자신의 move message인 경우
        pktMove.moveTime = m_moveTime;
        pktMove.processTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - m_serverProcessTime;
        pktMove.sendTime = m_sendTime;
        pktMove.recvTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
    }

    bw.Write(&pktMove, sizeof(PKT_S2C_MOVE));
    sendBuffer->OnWrite(sizeof(PKT_S2C_MOVE));

    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		RioServerSession::SendEnterMsg
*	Summary:	send enter message to client
*   Args:       int targetId
*                   target client id
*               unsigned short x
*                   position x
*               unsigned short y
*                   position y
-------------------------------------------------------- */
void RioServerSession::SendEnterMsg(int targetId, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_ENTER));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_ENTER pktEnter;
    pktEnter.header.id = PROTO_ID::S2C_ENTER;
    pktEnter.header.size = sizeof(PKT_S2C_ENTER);
    pktEnter.id = m_connectClientId;
    pktEnter.targetId = targetId;
    pktEnter.x = x;
    pktEnter.y = y;

    bw.Write(&pktEnter, sizeof(PKT_S2C_ENTER));

    sendBuffer->OnWrite(sizeof(PKT_S2C_ENTER));
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
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_LEAVE));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_LEAVE pktLeave;
    pktLeave.header.id = PROTO_ID::S2C_LEAVE;
    pktLeave.header.size = sizeof(PKT_S2C_LEAVE);
    pktLeave.id = m_connectClientId;
    pktLeave.targetId = targetId;

    bw.Write(&pktLeave, sizeof(PKT_S2C_LEAVE));

    sendBuffer->OnWrite(sizeof(PKT_S2C_LEAVE));
    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		RioServerSession::SendLeaveMsg
*	Summary:	send leave message to client
*   Args:       bool result
*                   login result
*               unsigned short x
*                   position x
*               unsigned short y
*                   position y
-------------------------------------------------------- */
void RioServerSession::SendLoginResult(bool result, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_LOGIN_RESULT));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_LOGIN_RESULT pktResult;
    pktResult.header.id = PROTO_ID::LOGIN_RESULT;
    pktResult.header.size = sizeof(PKT_S2C_LOGIN_RESULT);
    pktResult.result = result;
    pktResult.id = m_connectClientId;
    pktResult.x = x;
    pktResult.y = y;
    pktResult.loginTime = m_loginTime;

    bw.Write(&pktResult, sizeof(PKT_S2C_LOGIN_RESULT));

    sendBuffer->OnWrite(sizeof(PKT_S2C_LOGIN_RESULT));

    Send(sendBuffer);
}

#endif // RIO