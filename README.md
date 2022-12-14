# TCP/IP 소켓 프로그래밍
C/C++ 언어를 이용한 TCP 소켓 프로그래밍

### 0. 윈도우 소켓 소개
 여기서 사용하는 윈도우 소켓 버전은 WinSock2.2 버전이며 ws2_32.lib 를 링크하여 사용한다. 윈도우 소켓 라이브러리는 WinSock2.h 메인 헤더와 WS2tcpip.h 확장 헤더를 지원한다. WSADATA 구조체를 초기화(WSAStartup(ver, WSADATA*)하여 윈도우 소켓 사용을 선언하고 윈도우 소켓 사용을 종료할 때에는 WSACleanup() 함수를 이용한다.

### 1. TCP 서버/클라이언트 구현
 TCP/IP 소켓 프로그래밍을 수행하기 위해서는 클라이언트 사이드와 서버 사이드가 서버 사이드의 지역 IP 주소와 포트 번호를 알고 있어야 한다는 전제하에 진행된다. 그렇기 때문에 윈도우 소켓에서 지원하는 소켓 주소 구조체를 사용해야 한다. IPv4 프로토콜의 경우 sockaddr_in 구조체를 사용하고 IPv6 프로토콜의 경우 sockaddr_in6 구조체를 사용한다. 구조체의 크기만 다를 뿐 사용법은 동일하다. sockaddr_in 구조체 내부에는 sin_addr, sin_family, sin_port 변수가 있다. sin_addr 변수는 저장할 주소의 네트워크 바이트 정렬(빅 엔디안) 형식을 받는다. 그렇기 때문에 본인의 컴퓨터 환경이 리틀 엔디안일 경우 빅 엔디안으로 변환해주는 바이트 정렬 변환 함수를 사용해야 한다. 예를 들어 리틀 엔디안(이하 호스트 바이트 정렬) 환경의 경우 htonl() 및 htons() 함수를 사용해서 바이트 정렬 방식을 변환할 수 있다. 접미사인 l, s는 long, short를 의미하며 저장되어야할 크기에 따라 상황에 맞게 사용하면 된다. 주로 주소 형식에는 htonl 함수를, 포트 번호에는 htons 함수를 사용한다. 다시 sin_addr 변수에는 주소를 저장하고 sin_family 변수에는 주소 체계를 저장한다. IPv4 환경에서는 AF_INET 매크로를 사용하고 IPv6 환경에서는 AF_INET6 를 사용한다. sin_port 변수에는 서버 프로세스의 포트 번호를 입력하는데 앞에서 말했던것처럼 바이트 정렬 방식을 맞춰줘야 한다. 여기서는 htons() 함수를 주로 사용한다. 소켓 주소 구조체는 윈도우 소켓 프로그래밍에서 필수적으로 사용되기 때문에 자주 등장한다.

 SOCKET 구조체는 윈도우 소켓에서 지원하는 구조체이다. SOCKET 구조체를 활용하는 것이 이번 윈도우 네트워크 프로그래밍의 핵심이다. SOCKET 구조체를 사용하기 위해서는 먼저 윈도우 소켓을 사용하겠다고 미리 선언해야한다. SOCKET 구조체의 초기화 방법은 아래와 같다.
```c/c++
SOCKET sock = socket(af, type, protocol);
if (INVALID_SOCKET == sock) return 1;
```

 socket() 함수는 매개변수로 소켓에 적용할 주소 체계와 데이터 타입 그리고 전송 계층의 프로토콜을 선택할 수 있다. af 매개변수는 주소 체계로 IPv4 환경에서는 AF_INET 을 사용하고 IPv6 환경에서는 AF_INET6를 사용한다. type 매개변수는 전송할 데이터의 타입을 설정하는 것으로 볼 수 있는데 프로토콜에 따라 다르다. TCP의 경우 SOCK_STREAM 매크로를 사용하고 UDP의 경우 SOCK_DGRAM 매크로를 사용하면 된다. protocol 매개변수의 경우에는 IPPROTO_TCP, IPPROTO_UDP 와 같은 전송 단계의 프로토콜을 설정하는 것으로 앞의 type 변수가 SOCK_STREAM 일 경우 TCP를, SOCK_DGRAM 일 경우 UDP를 설정하면 된다. type 매개변수를 옳게 설정해두었다면 0으로 설정해주어도 무관하다. type 매개변수에 따라 TCP 또는 UDP 방식으로 구분이 가능하기 때문이다.

 기본적으로 소켓 프로그래밍은 큰 흐름이 있다. 1. 서버에서 대기(listen)하고 2. 클라이언트에서 서버에 연결(connect)하고 3. 클라이언트에서 송신(send)하고 4. 서버에서 수신(recv)하고 5. 클라이언트에서 종료(closesocket)하고 6. 서버도 종료(closesocket)한다. 이런 큰 흐름만 기억하고 있다면 TCP 소켓 프로그래밍은 한결 접근하기 쉬워질 것이다.

""TCP 코드 위치""
- TCPSever/*
- TCPClient/*

### 2. TCP 통신 데이터 전송 방식
 TCP 통신에서는 전송되는 데이터의 경계를 구분하지 않기 때문에 응용 프로그램 수준에서 메시지 경계를 구분하기 위한 추가 작업을 해야 한다. 네 가지 방법을 고려할 수 있다.

1. 송신자는 항상 고정 길이 데이터를 보내고, 수신자는 항상 고정 길이 데이터를 읽는다.
 구현이 쉽지만 데이터의 길이 변동폭이 클 경우 낭비되는 부분이 생기는 문제가 있다. 그러나 주고 받을 데이터의 길이 변동폭이 크지 않거나 제한되어 있는 경우라면 효과적인 방법이 될 수 있다.

2. 송신자는 가변 길이 데이터를 보내고 끝부분에 특별한 표시(EOR, End Of Record)를 붙인다.
 생성할 데이터의 길이를 미리 알 수 없을 때 적절하다. 데이터가 생성되면 곧바로 전송하되 끝에는 EOR을 전송한다. 이에 몇 가지 문제점이 있다. 먼저 데이터 중간에 EOR과 똑같은 패턴이 있으면 안 된다. 둘째로 데이터를 효율적으로 수신하는 방식을 고안하지 않으면 동작은 할지라도 수신자의 데이터 처리 속도가 지나치게 느려진다.

3. 송신자는 보낼 데이터 크기를 고정 길이 데이터로 보내고, 이어서 가변 길이 데이터를 보낸다. 수신자는 고정 길이 데이터를 읽어서 뒤따라올 가변 길이 데이터의 길이를 알아내고 이 길이만큼 데이터를 읽는다.
 생성할 데이터의 길이를 미리 알고 있는 상황에서 구현하기 쉽고 처리 효율성도 높다. 가변 길이 데이터를 수신할 때 recv 함수의 flag를 MSG_WAITALL로 그 길이의 모든 데이터를 읽을 수 있다.

4. 송신자는 가변 길이 데이터를 전송한 후 연결을 정상 종료한다. 수신자는 recv() 함수의 리턴값이 0이 될 때까지 데이터를 읽는다.
 한쪽에서 다른 쪽으로 일방적으로 가변 길이 데이터를 보낼 때 적절하다. 하지만 데이터를 자주 전송하는 경우에는 연결 처리를 반복해야하므로 비효율적이다.

### 3. 멀티 쓰레드를 이용한 TCP 서버/클라이언트

### 4. UDP 서버/클라이언트 구현

### 5. UDP 브로드캐스트 구현

### 6. UDP 멀티캐스트 구현

### 7. 소켓 옵션

### Project add implement details
- 서버에 디버깅 모드 추가 (/d 옵션)
- UDP 프로토콜 지원 
- 파일 전송 기능 추가 
- 채팅 ID 추가 
- 메시지 전송 방식 변경 추가
- 서버에 소켓 옵션 적용
- 새로운 클라이언트 접속 시 채팅 메시지 복원
- 서버에서 넌블로킹 소켓 사용
- 다양한 소켓 입출력 모델 적용

# References
- TCP/IP 소켓 프로그래밍/한빛아카데미 - 김선우 저서