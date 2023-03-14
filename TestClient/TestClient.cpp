#include "CorePch.h"
#include "pch.h"

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

    // Init Winsock
    WSADATA wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    // socket
    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        HandleError("socket");
        return 0;
    }

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr)); 
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(SERVERPORT);

    // connect
    if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
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
        len = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
        if (len == SOCKET_ERROR)
        {
            HandleError("Send");
            break;
        }

        cout << "Client | send data to server : " << len << " bytes" << endl;
        cout << "Client | data = " << sendBuffer << endl;


        len = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
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
    ::closesocket(clientSocket);

    // winsock close
    ::WSACleanup();
    return 0;
}
