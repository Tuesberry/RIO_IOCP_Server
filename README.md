# RIO & IOCP MMO Game Server
* 2023-1H KHU Software Convergence Capstone Design Project
* 주제 : Registered I/O 기반 Seamless MMO 게임서버 구현 및 성능 개선
* 프로젝트 보고서 : https://tuesberry.notion.site/Registered-I-O-based-Seamless-MMO-Game-Server-e7f2b21eabd64dea9c4ad946577143de
* [박경숙](https://github.com/Tuesberry)

## Description

RIO, IOCP 모델을 사용하여 개발한 게임 서버 프로그램입니다. </br>

플레이어의 이동과 채팅 동기화를 구현한 간단한 프로그램으로, RIO 서버와 IOCP 서버의 성능 비교를 위해 개발했습니다. 아직 완벽한 서버는 아니지만 개선해 나가는 중입니다.</br>

Core i7-10세대, Core 4개(스레드 8개), RAM 16GB 컴퓨터 기준, 5500개까지 동시접속이 가능합니다.

## Directory

| name     | description                                           |
| -------- | ------------------------------------------------------|
| Binary   | 빌드한 서버 exe 프로그램 directory                      |
| DummyClient | 더미 클라이언트 소스코드 directory                   |
| Libraries | ServerCore library, Protobuf library directory       |
| RIOIOCPServer  | RIO, IOCP Server 소스코드 directory              |
| ServerCore     | Server library directory                        |

## Start Guide

### Requirements
RIOIOCP 서버를 빌드하고 실행하기 위해서는 다음의 요구사항이 충족되어야 한다. </br>
서버 라이브러리만 활용하고자 한다면 꼭 설치하지 않아도 된다.

* [Protobuf 3.21.12](https://github.com/protocolbuffers/protobuf)

Protobuf를 빌드한 다음, `/Libraries/Libs/Protobuf` 아래에 빌드한 라이브러리 파일을 위치시킨다.</br>

```bash
└── Libraries
    └── Libs
        └── Protobuf
            ├── Debug
            |   ├── libprotobufd.dll 
            |   └── libprotobufd.lib
            └── Release
                ├── libprotobuf.dll 
                └── libprotobuf.lib
```

### Installation
```bash
$ git clone https://github.com/Tuesberry/RIO_IOCP_Server.git
```

### RIOIOCPServer 실행 방법
1. 먼저 ServerCore 라이브러리를 실행하고자 하는 환경(Debug/Release)에 맞춰 빌드한다. 정상적으로 빌드가 완료되었다면 `Libraries/Libs/ServerCore` 경로에 라이브러리가 있을 것이다.
2. DummyClient와 RIOIOCPServer를 빌드한 후에 실행한다.

## ServerCore Library 활용 방법

### ServerSession

서버 라이브러리의 세션을 상속하는 콘텐츠단의 세션 코드를 작성해야 한다.
IOCP 서버를 사용한다면 `IocpSession`, RIO 서버를 사용한다면 `RioSession`을 상속한다. 

```cpp
virtual void OnConnected(){}
virtual int OnRecv(char* buffer, int len) final;
virtual void OnRecvPacket(char* buffer, int len){}
virtual void OnSend(int len){}
virtual void OnDisconnected(){}
```

위 함수들은 콘텐츠 단에서 재정의해서 사용할 수 있다.

### ServerProxy

서버 프록시 클래스를 사용하여 간단하게 서버를 실행할 수 있다.
서버 타입과 세션 팩토리 함수는 같은 IO 모델로 통일되어야 한다. 

#### 서버 생성
서버 생성시 다음과 같은 인자들을 넘겨줘야 한다.

* ServerType : 서버 종류(IOCP_SERVER or RIO_SERVER)
* SockAddress : 서버의 ip 주소와 포트 넘버
* SessionFactory : 세션 팩토리 함수

#### 서버 실행 : Start
JobQueue 사용여부를 인자로 넘겨준다. 사용을 원하면 true, 원하지 않는다면 false

```cpp
int main()
{
    // 1. 서버 생성
    shared_ptr<ServerProxy> server = std::make_shared<ServerProxy>(
        ServerType::IOCP_SERVER,
        SockAddress("127.0.0.1", 7777),
        std::make_shared<IocpServerSession>
    );
    // 2. 서버 실행
    server->Start(true);
    // 3. 스레드 join 대기
    gThreadMgr->JoinThreads();
}
```

### 주의 사항

패킷 전송시, 직렬화된 패킷의 앞에는 PacketHeader가 위치해야 한다.
PacketHeader가 없으면, 전송받은 패킷을 역직렬화 할 수 없다.
```cpp
struct PacketHeader
{
	unsigned short size;
	unsigned short id; // protocol id 
};
```

## Reference
* 강수빈, (2020), RIO와 HTM을 이용한 게임서버의 성능 개선, 한국산업기술대학교
* 배현직, 게임 서버 프로그래밍 교과서, 길벗
* 김선우, TCP/IP 소켓 프로그래밍, 한빛아카데미
* [Rookiss, [C++과 언리얼로 만드는 MMORPG 게임 개발 시리즈] Part4: 게임 서버, 인프런](https://www.inflearn.com/course/%EC%96%B8%EB%A6%AC%EC%96%BC-3d-mmorpg-4)
* https://github.com/jacking75/edu_cpp_IOCP.git
