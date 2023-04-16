#include "RioServerSession.h"

RioServerSession::RioServerSession()
{
}

void RioServerSession::OnConnected()
{
    cout << "Connect!" << endl; 
}

int RioServerSession::OnRecv(char* buffer, int len)
{
    cout << "On Recv : " << buffer;
    cout << ", len : " << len << endl;
    return len;
}

void RioServerSession::OnSend(int len)
{
    cout << "On Send : " <<  len << endl;
}

void RioServerSession::OnDisconnected()
{
    cout << "Disconnect!" << endl;
}