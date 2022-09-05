#include "Common.h"

#define SERVERPORT	9000
#define BUFSIZE		512

DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE + 1];
	int retval;

	// 클라 정보 받기
	addrlen = sizeof(clientaddr);
	getpeername(sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1)
	{
		// 데이터 수신
		retval = recv(sock, buf, BUFSIZE, 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("recv()");
			break;
		}
		else if (0 == retval)
			break;

		// 데이터 출력
		buf[retval] = '\0';
		printf("[TCP/%s:%d] %s", addr, ntohs(clientaddr.sin_port), buf);

		// 데이터 송신
		retval = send(sock, buf, retval, 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("send()");
			break;
		}
	}

	closesocket(sock);
	printf("[TCP Server] 클라이언트 접속 종료: IP 주소 = %s, 포트 번호 = %d\n", addr, ntohs(clientaddr.sin_port));
	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	// 서버 지역 정보 할당
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_sock) err_quit("sock()");

	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("listen()");

	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	while (1)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (INVALID_SOCKET == client_sock)
		{
			err_display("accept()");
			break;
		}

		// 클라 접속 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP Server] 클라이언트 접속: IP 주소 = %s, 포트 번호 = %d\n", addr, ntohs(clientaddr.sin_port));

		// 멀티 스레드 돌리기
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, NULL, 0);
		if (NULL == hThread) closesocket(client_sock);
		else CloseHandle(hThread);
	}

	// 소켓 종료
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}