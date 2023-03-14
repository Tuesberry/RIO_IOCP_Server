#include <iostream>
#include <thread>
#include <vector>

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define SERVERPORT 7777
#define BUFSIZE 100

void HandleError(const char* cause)
{
    _int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;
}

struct Session
{
    SOCKET socket = INVALID_SOCKET;
    char Buffer[BUFSIZE] = {};
    int recvBytes = 0;
    int sendBytes = 0;
};

enum IO_TYPE
{
    READ,
    WRITE,
    ACCEPT,
    CONNECT,
};

struct OverlappedEx
{
    WSAOVERLAPPED overlapped = {};
    int type = 0; // read, write, accept, connect ...
};

void WorkerThreadMain(HANDLE iocpHandle)
{
    while (true)
    {
        DWORD bytesTransferred = 0;
        Session* session = nullptr;
        OverlappedEx* overlappedEx = nullptr;

        // GetQueuedCompletionStatus
        BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred,
            (ULONG_PTR*)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);

        // Get Client Info
        SOCKADDR_IN clientAddr;
        int addrlen = sizeof(clientAddr);
        ::getpeername(session->socket, (SOCKADDR*)&clientAddr, &addrlen);
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, addr, sizeof(addr));

        // result check
        if (ret == FALSE || bytesTransferred == 0)
        {
            // TODO : ¿¬°á ²÷±è
            closesocket(session->socket);
            continue;
        }

        if (overlappedEx->type == IO_TYPE::READ)
        {
            cout << "Recv Data IOCP = " << bytesTransferred << " bytes" << endl;
            cout << "Data = " << session->Buffer << endl;

            WSABUF wsaBuf;
            wsaBuf.buf = session->Buffer;
            wsaBuf.len = BUFSIZE;

            overlappedEx->type = IO_TYPE::WRITE;

            DWORD sendLen = 0;
            if(::WSASend(session->socket, &wsaBuf, 1, &sendLen, 0, &overlappedEx->overlapped, NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                {
                    HandleError("WSASend");
                    break;
                }
                continue;
            }

        }
        else if (overlappedEx->type == IO_TYPE::WRITE)
        {
            cout << "Send Data IOCP = " << bytesTransferred << " bytes" << endl;
            cout << "Data = " << session->Buffer << endl;

            memset(&overlappedEx->overlapped, 0, sizeof(overlappedEx->overlapped));
            memset(session->Buffer, 0, sizeof(session->Buffer));

            WSABUF wsaBuf;
            wsaBuf.buf = session->Buffer;
            wsaBuf.len = BUFSIZE;

            overlappedEx->type = IO_TYPE::READ;

            DWORD recvLen = 0;
            DWORD flags = 0;
            if(::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                {
                    HandleError("WSARecv");
                    break;
                }
                continue;
            }
        }

        
    }
}

int main()
{
    // Init Winsock
    WSADATA wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    // listen socket
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        HandleError("socket");
        return 0;
    }

    // bind
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SERVERPORT);

    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        HandleError("bind");
        return 0;
    }

    // listen
    if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        HandleError("listen");
        return 0;
    }

    // Create IoCompletionPort
    HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (iocpHandle == NULL)
    {
        HandleError("CreateIoCompletionPort");
        return 0;
    }

    // check number of cpus
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    // Create WorkerThreads
    vector<thread> threads;
    for (int i = 0; i < si.dwNumberOfProcessors * 2; i++)
    {
        threads.push_back(thread([=]() 
            {
                WorkerThreadMain(iocpHandle);
            }));
    }

    // session manager
    vector<Session*> sessionManager;

    // Main Thread = accpet()
    while (true)
    {
        // accept
        SOCKADDR_IN clientAddr;
        int addrLen = sizeof(clientAddr);

        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            HandleError("accept");
            break;
        }

        cout << "Client Connected" << endl;

        // session
        Session* session = new Session();
        session->socket = clientSocket;
        sessionManager.push_back(session);

        // socket, cp ¿¬°á
        ::CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)session, 0);

        // WSABuf
        WSABUF wsaBuf;
        wsaBuf.buf = session->Buffer;
        wsaBuf.len = BUFSIZE;

        // overlappedEX
        OverlappedEx* overlappedEx = new OverlappedEx();
        overlappedEx->type = IO_TYPE::READ;

        // WSARecv
        DWORD recvLen = 0;
        DWORD flags = 0; 
        if (::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != ERROR_IO_PENDING)
            {
                HandleError("WSARecv");
                break;
            }
            continue;
        }
    }

    // thread join
    for (int i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }

    // winsock close
    ::WSACleanup();
    return 0;
}
