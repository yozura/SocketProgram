#include "Common.h"

#define SERVERPORT	9000
#define BUFSIZE		512

int main()
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == sock) err_quit("sock()");
	
	retval = bind(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	// 클라 정보
	struct sockaddr_in clientaddr;
	char buf[BUFSIZE + 1];
	int addrlen;

	while (1)
	{
		addrlen = sizeof(clientaddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, &addrlen);
		if (SOCKET_ERROR == retval)
		{
			err_display("recvfrom()");
			break;
		}

		// 메시지 출력
		buf[retval] = '\0';
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[UDP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);
		
		retval = sendto(sock, buf, retval, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
		if (SOCKET_ERROR == retval)
		{
			err_display("sendto()");
			break;
		}
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}