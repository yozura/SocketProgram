#include "Common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT	9000
#define BUFSIZE		512

int main(int argc, char* argv[])
{
	int retval;

	if (argc > 1) SERVERIP = argv[1];

	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == sock) err_quit("sock()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);

	struct sockaddr_in peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;

	while (1)
	{
		printf("\n[보낼 데이터] ");
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

		printf("[UDP Client] %dbyte를 보냈습니다.\n", retval);

		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&peeraddr, &addrlen);
		if (SOCKET_ERROR == retval)
		{
			err_display("recvfrom()");
			break;
		}

		// IP 주소 체크
		if (memcmp(&peeraddr, &serveraddr, sizeof(peeraddr)))
		{
			printf("[오류] 잘못된 데이터입니다!\n");
			break;
		}

		buf[retval] = '\0';
		printf("[UDP Client] %dbyte를 받았습니다.\n", retval);
		printf("[받은 데이터] %s\n", buf);
	}

	closesocket(sock);

	WSACleanup();
	return 0;
}