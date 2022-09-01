#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE 512

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSAData wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	// ���� ����(������ ����) ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_sock) err_quit("sock()");

	// ���� ���� ���ε�
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	// ���� ���� ������
	retval = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("listen()");

	// Ŭ���̾�Ʈ ���� ���� �غ�
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int clientaddr_len;
	char buf[BUFSIZE + 1];

	// ���� ���� ���
	while (1)
	{
		clientaddr_len = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &clientaddr_len);
		if (INVALID_SOCKET == client_sock)
		{
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, INET_ADDRSTRLEN);
		printf("\n[TCP Server] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n", addr, ntohs(clientaddr.sin_port));

		// ���� ���� �ۼ���
		while (1)
		{
			// ������ ����
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (SOCKET_ERROR == retval)
			{
				err_display("recv()");
				break;
			}
			// ���� ���� ��
			else if (retval == 0)
				break;

			// ������ ���
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

			// ������ �۽�
			retval = send(client_sock, buf, strlen(buf), 0);
			if (SOCKET_ERROR == retval)
			{
				err_display("send()");
				break;
			}
		}

		closesocket(client_sock);
		printf("[TCP Server] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n", addr, ntohs(clientaddr.sin_port));
	}

	// ���� ����(������ ����) ����
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}