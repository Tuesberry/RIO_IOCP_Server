#include "CorePch.h"
#include "pch.h"
#include "Network/SocketCore.h"

#define SERVERPORT 7777
#define BUFSIZE 100

void HandleError(const char* cause)
{
    _int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;
}

int main()
{
    this_thread::sleep_for(1s);

    SocketCore sockCore;

    if (sockCore.Init() == false)
        return 0;

    if (sockCore.Socket() == false)
    {
        HandleError("socket");
        return 0;
    }

    SockAddress sockAddress("127.0.0.1", SERVERPORT);
    if (sockCore.Connect(sockAddress) == false)
    {
        HandleError("Connect");
        return 0;
    }

    cout << "Conencted to Server!!" << endl;

    char sendBuffer[BUFSIZE] = "Hello World";
    char recvBuffer[BUFSIZE];
    int len;

    // send and recv
    while (true)
    {
        len = sockCore.Send(sendBuffer, sizeof(sendBuffer));
        if (len == SOCKET_ERROR)
        {
            HandleError("Send");
            break;
        }

        cout << "Client | send data to server : " << len << " bytes" << endl;
        cout << "Client | data = " << sendBuffer << endl;


        len = sockCore.Recv(recvBuffer, sizeof(recvBuffer));
        if (len == SOCKET_ERROR)
        {
            HandleError("Recv");
            break;
        }

        cout << "Client | recv data from server : " << len << " bytes" << endl;
        cout << "Client | data = " << recvBuffer << endl;
        
        this_thread::sleep_for(1s);

    }

    // close socket
    sockCore.Close();

    // winsock close
    sockCore.Clear();

    return 0;
}
