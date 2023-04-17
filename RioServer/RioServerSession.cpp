#include "RioServerSession.h"

RioServerSession::RioServerSession()
{
    cout << "RioSession" << endl;
}

void RioServerSession::OnConnected()
{
    cout << "Connect!" << endl; 
}

void RioServerSession::OnRecvPacket(char* buffer, int len)
{
    cout << buffer << " " << len << endl;

    Send(buffer, len);
}

void RioServerSession::OnSend(int len)
{
    cout << "On Send : " <<  len << endl;
}

void RioServerSession::OnDisconnected()
{
    cout << "Disconnect!" << endl;
}