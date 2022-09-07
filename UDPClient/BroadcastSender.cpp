#include "Common.h"

#define REMOTEIP	"255.255.255.255"
#define REMOTEPORT	9000
#define BUFSIZE		512

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	struct sockaddr_in remoteaddr;
	memset(&remoteaddr, 0, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;
	inet_pton(AF_INET, REMOTEIP, &remoteaddr.sin_addr);
	remoteaddr.sin_port = htons(REMOTEPORT);

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == sock) err_quit("sock()");

	DWORD bEnable = 1;
	retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&bEnable, sizeof(bEnable));
	if (SOCKET_ERROR == retval) err_quit("setsockopt()");

	char buf[BUFSIZE + 1];
	int len;
	
	while (1)
	{
		printf("\n[보낼 데이터] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// 뉴라인제거
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		retval = sendto(sock, buf, (int)strlen(buf), 0, (struct sockaddr*)&remoteaddr, sizeof(remoteaddr));
		if (SOCKET_ERROR == retval)
		{
			err_display("sendto()");
			break;
		}

		printf("[UDP] %dByte를 보냈습니다.\n", retval);
	}

	closesocket(sock);
	 
	WSACleanup();
	return 0;
}