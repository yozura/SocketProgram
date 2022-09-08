#include "Common.h"

#define SERVERIP (char*)"127.0.0.1"
#define SERVERPORT	9000
#define BUFSIZE		50

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sock) err_quit("sock()");

	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("connect()");

	// ������ ��ſ� �ʿ��� ������
	char buf[BUFSIZE + 1];

	while (1)
	{
		// ������ �Է�
		printf("[���� ������] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// ������ �۽�
		retval = send(sock, buf, (int)(strlen(buf)), 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("send()");
			break;
		}

		printf("[TCP Client] %dByte�� ���½��ϴ�.\n", retval);

		// ������ ����
		retval = recv(sock, buf, retval, MSG_WAITALL);
		if (SOCKET_ERROR == retval)
		{
			err_display("recv()");
			break;
		}
		else if (0 == retval)
			break;

		buf[retval] = '\0';
		printf("[TCP Client] %dByte�� �޾ҽ��ϴ�.\n", retval);
		printf("[���� ������] %s\n", buf);
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}