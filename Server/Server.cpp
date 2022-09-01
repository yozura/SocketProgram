#include "Common.h"

#define SERVERPORT	9000
#define BUFSIZE		512

DWORD WINAPI TCPServer4(LPVOID arg)
{
	int retval;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// 바인딩
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	while (1) 
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		printf("\n[TCP Server] 클라이언트 접속 : IP 주소 = %s, 포트 번호 = %d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1)
		{
			// 데이터 받기 
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			printf("%s", buf);
		}

		// 소켓 닫기
		closesocket(client_sock);
		printf("[TCP Server] 클라이언트 종료 : IP 주소 = %s, 포트 번호 = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(listen_sock);
	return 0;
}

DWORD WINAPI TCPServer6(LPVOID arg)
{
	int retval;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// 듀얼 스택을 끈다. [Windows는 꺼져 있음(기본값), UNIX/Linux는 OS마다 다름]
	int no = 1;
	setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&no, sizeof(no));

	// bind()
	struct sockaddr_in6 serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin6_family = AF_INET6;
	serveraddr.sin6_addr = in6addr_any;
	serveraddr.sin6_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in6 clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	while (1) 
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char ipaddr[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &clientaddr.sin6_addr, ipaddr, sizeof(ipaddr));
		printf("\n[TCP Server] 클라이언트 접속 : IP 주소 = %s, 포트 번호 = %d\n",
			ipaddr, ntohs(clientaddr.sin6_port));

		// 클라이언트와 데이터 통신
		while (1) 
		{
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0) break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			printf("%s", buf);
		}

		// 소켓 닫기
		closesocket(client_sock);
		printf("[TCP Server] 클라이언트 종료 : IP 주소 = %s, 포트 번호 = %d\n",
			ipaddr, ntohs(clientaddr.sin6_port));
	}

	closesocket(client_sock);
	return 0;
}

/*
int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 멀티스레드를 이용하여 두 개의 서버를 동시 구현한다.
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, TCPServer4, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, TCPServer6, NULL, 0, NULL);
	
	// nCount : lpHandles가 가리키는 배열의 개체 핸들 수. 개수 제한은 0 < nCount <= MAXIMUM_WAIT_OBJECTS
	// lpHandles : 개체 핸들의 배열.
	// bWaitAll : 이 매개변수가 TRUE면 lpHandles 배열에 있는 모든 객체의 상태가 신호를 받을 때 함수가 반환된다.
	// FALSE인 경우 객체 중 하나의 상태가 신호로 설정되면 함수가 반환된다. 이 경우 반환 값은 해당 상태로 인해 함수가 반환된 개체를 나타낸다.
	// dwMilliseconds : 0이 아닌 값이 지정되면 함수는 지정된 객체에 신호가 전달되거나 간격이 경과할 때까지 대기한다.
	// 0이면 객체가 신호를 받지 않을 경우 함수가 대기 상태로 들어가지 않고 즉시 반환됨.
	// INFINITE 일 경우 지정된 객체가 신호를 받을 때만 함수가 반환.
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	// 윈속 종료
	WSACleanup();
	return 0;
}
*/