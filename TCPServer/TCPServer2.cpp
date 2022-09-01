#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE 512

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSAData wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	// 서버 소켓(리스닝 소켓) 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_sock) err_quit("sock()");

	// 서버 소켓 바인딩
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	// 서버 소켓 리스닝
	retval = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("listen()");

	// 클라이언트 소켓 받을 준비
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int clientaddr_len;
	char buf[BUFSIZE + 1];

	// 서버 소켓 대기
	while (1)
	{
		clientaddr_len = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &clientaddr_len);
		if (INVALID_SOCKET == client_sock)
		{
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, INET_ADDRSTRLEN);
		printf("\n[TCP Server] 클라이언트 접속 : IP 주소 = %s, 포트 번호 = %d\n", addr, ntohs(clientaddr.sin_port));

		// 서버 소켓 송수신
		while (1)
		{
			// 데이터 수신
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (SOCKET_ERROR == retval)
			{
				err_display("recv()");
				break;
			}
			// 정상 종료 시
			else if (retval == 0)
				break;

			// 데이터 출력
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

			// 데이터 송신
			retval = send(client_sock, buf, strlen(buf), 0);
			if (SOCKET_ERROR == retval)
			{
				err_display("send()");
				break;
			}
		}

		closesocket(client_sock);
		printf("[TCP Server] 클라이언트 종료 : IP 주소 = %s, 포트 번호 = %d\n", addr, ntohs(clientaddr.sin_port));
	}

	// 서버 소켓(리스닝 소켓) 종료
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}