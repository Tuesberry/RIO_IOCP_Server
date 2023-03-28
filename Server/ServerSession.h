#pragma once
#include "pch.h"
#include "CoreCommon.h"
#include "Network/IocpSession.h"

class ServerSession : public IocpSession
{
public:
    ~ServerSession()
    {
        cout << "~ServerSession" << endl;
    }

    virtual void OnRecvPacket(BYTE* buffer, int len) override
    {
        // Echo
        cout << "OnRecv Len = " << len << endl;
        cout << "Data = " << buffer[len] << endl;

        shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(len);
        ::memcpy(sendBuffer->GetData(), &buffer, sizeof(buffer));
        sendBuffer->OnWrite(len);
        Send(sendBuffer);
    }

    virtual void OnSend(int len) override
    {
        cout << "OnSend Len = " << len << endl;
    }
};