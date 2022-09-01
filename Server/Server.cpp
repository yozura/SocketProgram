#include "Common.h"

#define SERVERPORT	9000
#define BUFSIZE		512

DWORD WINAPI TCPServer4(LPVOID arg)
{
	int retval;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// ���ε�
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	while (1) 
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		printf("\n[TCP Server] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ���
		while (1)
		{
			// ������ �ޱ� 
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// ���� ������ ���
			buf[retval] = '\0';
			printf("%s", buf);
		}

		// ���� �ݱ�
		closesocket(client_sock);
		printf("[TCP Server] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// ���� �ݱ�
	closesocket(listen_sock);
	return 0;
}

DWORD WINAPI TCPServer6(LPVOID arg)
{
	int retval;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// ��� ������ ����. [Windows�� ���� ����(�⺻��), UNIX/Linux�� OS���� �ٸ�]
	int no = 1;
	setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&no, sizeof(no));

	// bind()
	struct sockaddr_in6 serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin6_family = AF_INET6;
	serveraddr.sin6_addr = in6addr_any;
	serveraddr.sin6_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	struct sockaddr_in6 clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	while (1) 
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char ipaddr[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &clientaddr.sin6_addr, ipaddr, sizeof(ipaddr));
		printf("\n[TCP Server] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			ipaddr, ntohs(clientaddr.sin6_port));

		// Ŭ���̾�Ʈ�� ������ ���
		while (1) 
		{
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0) break;

			// ���� ������ ���
			buf[retval] = '\0';
			printf("%s", buf);
		}

		// ���� �ݱ�
		closesocket(client_sock);
		printf("[TCP Server] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n",
			ipaddr, ntohs(clientaddr.sin6_port));
	}

	closesocket(client_sock);
	return 0;
}

/*
int main(int argc, char* argv[])
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ��Ƽ�����带 �̿��Ͽ� �� ���� ������ ���� �����Ѵ�.
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, TCPServer4, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, TCPServer6, NULL, 0, NULL);
	
	// nCount : lpHandles�� ����Ű�� �迭�� ��ü �ڵ� ��. ���� ������ 0 < nCount <= MAXIMUM_WAIT_OBJECTS
	// lpHandles : ��ü �ڵ��� �迭.
	// bWaitAll : �� �Ű������� TRUE�� lpHandles �迭�� �ִ� ��� ��ü�� ���°� ��ȣ�� ���� �� �Լ��� ��ȯ�ȴ�.
	// FALSE�� ��� ��ü �� �ϳ��� ���°� ��ȣ�� �����Ǹ� �Լ��� ��ȯ�ȴ�. �� ��� ��ȯ ���� �ش� ���·� ���� �Լ��� ��ȯ�� ��ü�� ��Ÿ����.
	// dwMilliseconds : 0�� �ƴ� ���� �����Ǹ� �Լ��� ������ ��ü�� ��ȣ�� ���޵ǰų� ������ ����� ������ ����Ѵ�.
	// 0�̸� ��ü�� ��ȣ�� ���� ���� ��� �Լ��� ��� ���·� ���� �ʰ� ��� ��ȯ��.
	// INFINITE �� ��� ������ ��ü�� ��ȣ�� ���� ���� �Լ��� ��ȯ.
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	// ���� ����
	WSACleanup();
	return 0;
}
*/