#include "Common.h"

char* SERVERIP = (char*)"127.0.0.1";

#define SERVERPORT	9000
#define BUFSIZE		50

int main(int argc, char* argv[])
{
	int retval;

	if (argc > 1) SERVERIP = argv[1];

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	char buf[BUFSIZE];
	const char* testdata[] = {
		"안녕하세요",
		"반가워요",
		"오늘따라 할 이야기가 많을 것 같네요.",
		"저도 그렇네요",
	};
	int len;

	for (int i = 0; i < 4; ++i)
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == sock) err_quit("sock()");

		retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		if (SOCKET_ERROR == retval) err_quit("quit()");

		// 데이터 입력
		len = (int)strlen(testdata[i]);
		strncpy(buf, testdata[i], len);

		// 데이터 송신
		retval = send(sock, buf, len, 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("send()");
			break;
		}

		printf("[TCP Client] %d바이트를 보냈습니다.\n", retval);
		
		closesocket(sock);
	}


	WSACleanup();
	return 0;
}