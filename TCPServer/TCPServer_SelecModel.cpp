#include "Common.h"

#define SERVERPORT	9000
#define BUFSIZE		512

// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO
{
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
};

int nTotalSockets = 0;
SOCKETINFO* SocketInfoArray[FD_SETSIZE];

// 소켓 정보 관리 함수
bool AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_sock) err_quit("socket()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("listen()");

	u_long on = 1;
	retval = ioctlsocket(listen_sock, FIONBIO, &on);
	if (SOCKET_ERROR == retval) err_display("ioctlsocket()");

	fd_set rSet, wSet;
	int nReady = 1;
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;

	while (1)
	{
		// 소켓 셋 초기화
		FD_ZERO(&rSet);
		FD_ZERO(&wSet);
		FD_SET(listen_sock, &rSet);
		for (int i = 0; i < nTotalSockets; ++i) 
		{
			if (SocketInfoArray[i]->recvbytes > SocketInfoArray[i]->sendbytes)
				FD_SET(SocketInfoArray[i]->sock, &wSet);
			else
				FD_SET(SocketInfoArray[i]->sock, &rSet);
		}

		// select()
		retval = select(0, &rSet, &wSet, NULL, NULL);
		if (SOCKET_ERROR == retval) err_quit("select()");

		// 소켓 셋 검사(1) : 클라이언트 접속 수용
		if (FD_ISSET(listen_sock, &rSet))
		{
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
			if (INVALID_SOCKET == client_sock)
			{
				err_display("accept()");
				break;
			}
			else
			{
				// 클라이언트 정보 출력
				char addr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
				printf("\n[TCP 서버] 클라이언트 접속 : IP 주소 = %s, 포트 번호 = %d\n", addr, ntohs(clientaddr.sin_port));

				// 소켓 정보 추가: 실패 시 소켓 닫기
				if (!AddSocketInfo(client_sock))
					closesocket(client_sock);
			}

			if (--nReady <= 0)
				continue;
		}

		// 소켓 셋 검사(2) : 데이터 통신
		for (int i = 0; i < nTotalSockets; ++i)
		{
			SOCKETINFO* ptr = SocketInfoArray[i];
			if (FD_ISSET(ptr->sock, &rSet))
			{
				// 데이터 받기
				retval = recv(ptr->sock, ptr->buf, BUFSIZE, 0);
				if (SOCKET_ERROR == retval)
				{
					err_display("recv()");
					RemoveSocketInfo(i);
				}
				else if (0 == retval)
					RemoveSocketInfo(i);
				else
				{
					ptr->recvbytes = retval;
					addrlen = sizeof(clientaddr);
					getpeername(ptr->sock, (struct sockaddr*)&clientaddr, &addrlen);
					
					ptr->buf[ptr->recvbytes] = '\0';
					char addr[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
					printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), ptr->buf);
				}
			}

			if (FD_ISSET(ptr->sock, &wSet))
			{
				// 데이터 보내기
				retval = send(ptr->sock, ptr->buf + ptr->sendbytes, ptr->recvbytes - ptr->sendbytes, 0);
				if (SOCKET_ERROR == retval)
				{
					err_display("send()");
					RemoveSocketInfo(i);
				}
				else
				{
					ptr->sendbytes += retval;
					if (ptr->recvbytes == ptr->sendbytes)
						ptr->recvbytes = ptr->sendbytes = 0;
				}
			}
		}
	}
	
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}

bool AddSocketInfo(SOCKET sock)
{
	if (nTotalSockets >= FD_SETSIZE)
	{
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return false;
	}

	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL)
	{
		printf("[오류] 메모리가 부족합니다!\n");
		return false;
	}

	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->sendbytes = 0;
	SocketInfoArray[nTotalSockets++] = ptr;
	return true;
}

void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO* ptr = SocketInfoArray[nIndex];

	struct sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (struct sockaddr*)&clientaddr, &addrlen);

	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	printf("[TCP 서버] 클라이언트 종료 : IP 주소 = %s, 포트 번호 = %d\n", addr, ntohs(clientaddr.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];
	--nTotalSockets;
}