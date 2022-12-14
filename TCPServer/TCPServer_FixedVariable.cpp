#include "Common.h"

#define SERVERPORT	9000
#define BUFSIZE		512

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_sock) err_quit("sock()");

	// 소켓 지역 주소, 지역 포트 할당
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	// 소켓 바인딩
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	// 소켓 리스닝
	retval = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int fixedLen;

	// 소켓 통신
	while (1)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (INVALID_SOCKET == client_sock)
		{
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP 서버] 클라이언트 접속 : IP 주소 = %s, 포트 번호 = %d\n", addr, ntohs(clientaddr.sin_port));

		while (1)
		{
			// 고정 길이 데이터 수신
			retval = recv(client_sock, (char*)&fixedLen, sizeof(fixedLen), MSG_WAITALL);
			if (SOCKET_ERROR == retval)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 가변 길이 데이터 수신
			retval = recv(client_sock, buf, fixedLen, MSG_WAITALL);
			if (SOCKET_ERROR == retval)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 정보 출력
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);
		}

		// 소켓 닫기
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료 : IP 주소 = %s, 포트 번호 = %d\n", addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 종료
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}