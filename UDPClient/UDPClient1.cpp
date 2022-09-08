#include "Common.h"

#define SERVERIP	(char*)"127.0.0.1"
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
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (SOCKET_ERROR == sock) err_quit("sock()");

	char buf[BUFSIZE + 1];
	int addrlen;
	int len;

	while (1)
	{
		printf("\n[보낼 메시지] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// '\n' 문자 제거
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		retval = sendto(sock, buf, (int)strlen(buf), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		if (SOCKET_ERROR == retval)
		{
			err_display("sendto()");
			break;
		}

		printf("[보낸 데이터] %dByte를 보냈습니다.\n", retval);

		addrlen = sizeof(serveraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&serveraddr, &addrlen);
		if (SOCKET_ERROR == retval)
		{
			err_display("recvfrom()");
			break;
		}

		// 메시지 출력
		buf[retval] = '\0';
		printf("[받은 데이터] %dByte를 받았습니다.\n", retval);
		printf("[받은 메시지] %s\n", buf);
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}