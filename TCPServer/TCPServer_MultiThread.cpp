#include "Common.h"

#define SERVERPORT	9000
#define BUFSIZE		512

DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE + 1];
	int retval;

	// Ŭ�� ���� �ޱ�
	addrlen = sizeof(clientaddr);
	getpeername(sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1)
	{
		// ������ ����
		retval = recv(sock, buf, BUFSIZE, 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("recv()");
			break;
		}
		else if (0 == retval)
			break;

		// ������ ���
		buf[retval] = '\0';
		printf("[TCP/%s:%d] %s", addr, ntohs(clientaddr.sin_port), buf);

		// ������ �۽�
		retval = send(sock, buf, retval, 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("send()");
			break;
		}
	}

	closesocket(sock);
	printf("[TCP Server] Ŭ���̾�Ʈ ���� ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n", addr, ntohs(clientaddr.sin_port));
	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	// ���� ���� ���� �Ҵ�
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_sock) err_quit("sock()");

	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("listen()");

	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	while (1)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (INVALID_SOCKET == client_sock)
		{
			err_display("accept()");
			break;
		}

		// Ŭ�� ���� ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP Server] Ŭ���̾�Ʈ ����: IP �ּ� = %s, ��Ʈ ��ȣ = %d\n", addr, ntohs(clientaddr.sin_port));

		// ��Ƽ ������ ������
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, NULL, 0);
		if (NULL == hThread) closesocket(client_sock);
		else CloseHandle(hThread);
	}

	// ���� ����
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}