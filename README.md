# Registered I/O 기반 MMORPG 게임 서버 구현 및 성능 개선
* 2023-1H KHU Software Convergence Capstone Design Project
* [박경숙](https://github.com/Tuesberry)

## Overview
* 온라인 MMO 게임 서버는 다수의 플레이어 간에 주고받는 데이터 양이 많다.
따라서 일정한 응답시간을 보장해주기위해서 운영체제에서 제공하는 고성능 네트워크 API를
사용해야 한다.
* Windows 서버에서는 주로 IOCP를 사용하여 게임 서버를 개발해왔지만, 더 성능이 우수한 RIO가 등장했다. RIO는 커널과 어플리케이션이 공유하는 자료구조를 두어, 완료 통지의 폴링을 시스템 콜이 아닌 유저 모드에서 구현함으로써 더 좋은 성능을 낸다.
* 이번 프로젝트에서 기존의 논문을 참고하여 RIO 서버를 개발하고, 딜레이가 발생하는 부분을 개선했다. 그리고 개발한 RIO서버와 IOCP서버에 대해 스트레스 테스트를 통한 성능 평가를 진행했다.

## RIO Server Architecture
![rio architecture](https://github.com/Tuesberry/RIO_IOCP_Server/assets/75127144/2a7cbb9e-c85d-4284-a49c-b221ffcd03ab)
* RequestQueue(RQ)와 CompletionQueue(CQ)를 담당하는 스레드를 할당하여 Race Condition을 제거함
* RioCore는 각각 담당하는 RioSession 리스트를 가지며, 해당 세션들에 대한 I/O만 처리함

![workerThread](https://github.com/Tuesberry/RIO_IOCP_Server/assets/75127144/24b9245a-8e46-4dde-8edf-f82929fa902e)
* WorkerThread는 Send Queue Operation과 Packet Process를 반복하여 수행함
* Send Queue Operation
  * Request Queue에서의 race condition 제거를 위해 한 번에 전송하는 방식을 사용함. 해당 클라이언트에게 전송할 메시지를 큐에 넣어두었다가, 한 번에 전송함
  * Deferred Send를 통해, Send 요청을 RQ에 모아두었다가 Send Commit을 통해 RQ에 모인 패킷을 한 번에 전송하여 System Call 호출 횟수를 줄임

## Stress Test
* 스트레스 테스트는 실제 게임에서 가장 부하가 많이 발생하는 이동 및 시야처리를 반복하는 방식으로 진행함
* 1s 내에 연결된 모든 클라이언트가 순차적으로 서버에 패킷을 전송하고 그로부터 응답을 받은 시간에 대한 평균치를 구해서 딜레이 측정
  
![stress test howto](https://github.com/Tuesberry/RIO_IOCP_Server/assets/75127144/14665c54-dcf0-4a8f-aa6b-5a602577a7ac)

1. 유저의 이동 방향 정보를 클라이언트에서 서버로 전송
2. 서버에서 유저의 새로운 위치 계산
3. 해당 유저와 일정 거리 내에 위치한 다른 플레이어들을 찾음
4. 각 유저들에게 서로의 위치 정보를 보내줘서, 동기화를 진행함

## RIO Performance Improvement
### Delay 원인 파악
* 클라이언트에게 전송할 메시지가 개별적으로 큐에 들어가 딜레이 발생 & 큐 공간 부족 문제 발생
* 데이터를 전송 시, 전송할 메시지들이 개별적으로 RQ에 들어가며 전송 완료에 대한 통지를 받을 때도 CQ에 해당 데이터들이 개별적으로 들어가 공간을 차지해 딜레이가 발생함

### Delay 개선 방법
![improvement](https://github.com/Tuesberry/RIO_IOCP_Server/assets/75127144/03d3ad26-a361-4caf-8425-4b51ee516285)
* 전송할 메시지를 여러개 모아 하나의 RIO_BUF로 전송함
* 전송할 메시지들을 버퍼에 넣어놓고, 버퍼에 포함된 데이터의 길이가 특정 바이트 이상이 되면, 그때 데이터를 전송하도록 함

## Result
* 클라이언트 개수 6000개까지 10개 단위로 진행
* 딜레이가 100ms에 도달하는 지점을 최대 동시 접속자 수로 판단함

### Server Delay(ms) 측정 결과
![output](https://github.com/Tuesberry/RIO_IOCP_Server/assets/75127144/14269cc3-2f64-4bd3-be07-a5c744bdba07)

### 최대 동시 접속자 수
|IOCP|RIO|RIO Improved|
|:---:|:---:|:---:|
|5230|4810|5530|

### Conclusion
* IOCP에서 RIO로 변경했을 때, 최대 동시 접속자 수는 5.74% 증가함

## Reference
* 강수빈, (2020), RIO와 HTM을 이용한 게임서버의 성능 개선, 한국산업기술대학교
* 배현직, 게임 서버 프로그래밍 교과서, 길벗
* 김선우, TCP/IP 소켓 프로그래밍, 한빛아카데미
* Rookiss, [C++과 언리얼로 만드는 MMORPG 게임 개발 시리즈] Part4: 게임 서버, 인프런, https://www.inflearn.com/course/%EC%96%B8%EB%A6%AC%EC%96%BC-3d-mmorpg-4
* https://github.com/jacking75/edu_cpp_IOCP.git
