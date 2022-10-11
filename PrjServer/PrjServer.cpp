#include "PrjServer.h"

#define SERVERPORT	9000
#define BUFSIZE		256

typedef struct _SOCKETINFO
{
	SOCKET sock;
	bool isIPv6;
	bool isUDP;
	char buf[BUFSIZE + 1];
	int recvbytes;
} SOCKETINFO;

int nTotalSockets = 0;
SOCKETINFO* SocketInfoArray[FD_SETSIZE];

// ���� ���� ���� �Լ�
bool AddSocketInfo(SOCKET sock, bool isIPv6, bool isUDP);
void RemoveSocketInfo(int nIndex);

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	/* ----- TCP/IPv4 ���� �ʱ�ȭ ���� -----*/
	SOCKET listen_sock4 = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_sock4) err_quit("socket()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);

	retval = bind(listen_sock4, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	retval = listen(listen_sock4, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("listen()");
	/* ----- TCP/IPv4 ���� �ʱ�ȭ ���� -----*/

	/* ----- TCP/IPv6 ���� �ʱ�ȭ ���� -----*/
	SOCKET listen_sock6 = socket(AF_INET6, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_sock6) err_quit("socket()6");

	struct sockaddr_in6 serveraddr6;
	memset(&serveraddr6, 0, sizeof(serveraddr6));
	serveraddr6.sin6_addr = in6addr_any;
	serveraddr6.sin6_family = AF_INET6;
	serveraddr6.sin6_port = htons(SERVERPORT);

	retval = bind(listen_sock6, (struct sockaddr*)&serveraddr6, sizeof(serveraddr6));
	if (SOCKET_ERROR == retval) err_quit("bind()6");

	retval = listen(listen_sock6, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("listen()6");
	/* ----- TCP/IPv6 ���� �ʱ�ȭ ���� -----*/

	/* ----- UDP/IPv4 ���� �ʱ�ȭ ���� -----*/
	SOCKET listen_sock_UDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == listen_sock_UDP) err_quit("socket()UDP");

	struct sockaddr_in serveraddr_UDP;
	memset(&serveraddr_UDP, 0, sizeof(serveraddr_UDP));
	serveraddr_UDP.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_UDP.sin_family = AF_INET;
	serveraddr_UDP.sin_port = htons(SERVERPORT);

	retval = bind(listen_sock_UDP, (struct sockaddr*)&serveraddr_UDP, sizeof(serveraddr_UDP));
	if (SOCKET_ERROR == retval) err_quit("bind()UDP");
	/* ----- UDP/IPv4 ���� �ʱ�ȭ ���� -----*/

	/* ----- UDP/IPv6 ���� �ʱ�ȭ ���� -----*/
	SOCKET listen_sock_UDP6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == listen_sock_UDP6) err_quit("socket()UDP6");

	struct sockaddr_in6 serveraddr_UDP6;
	memset(&serveraddr_UDP6, 0, sizeof(serveraddr_UDP6));
	serveraddr_UDP6.sin6_addr = in6addr_any;
	serveraddr_UDP6.sin6_family = AF_INET6;
	serveraddr_UDP6.sin6_port = htons(SERVERPORT);

	retval = bind(listen_sock_UDP6, (struct sockaddr*)&serveraddr_UDP6, sizeof(serveraddr_UDP6));
	if (SOCKET_ERROR == retval) err_quit("bind()UDP6");
	/* ----- UDP/IPv6 ���� �ʱ�ȭ ���� -----*/

	// ������ ��ſ� ����� ����(����)
	fd_set rset;
	SOCKET client_sock;
	int addrlen;

	// ������ ��ſ� ����� ����(����)
	struct sockaddr_in clientaddr4;
	struct sockaddr_in6 clientaddr6;

	while (true) 
	{
		// ���� �� �ʱ�ȭ
		FD_ZERO(&rset);
		FD_SET(listen_sock4, &rset);
		FD_SET(listen_sock6, &rset);
		FD_SET(listen_sock_UDP, &rset);
		FD_SET(listen_sock_UDP6, &rset);
		for (int i = 0; i < nTotalSockets; ++i)
			FD_SET(SocketInfoArray[i]->sock, &rset);

		// select()
		retval = select(0, &rset, NULL, NULL, NULL);
		if (SOCKET_ERROR == retval) err_quit("select()");

		// ���� �� �˻�(1) : Ŭ���̾�Ʈ ���� ����
		if (FD_ISSET(listen_sock4, &rset))
		{
			addrlen = sizeof(clientaddr4);
			client_sock = accept(listen_sock4, (struct sockaddr*)&clientaddr4, &addrlen);
			if (INVALID_SOCKET == client_sock)
			{
				err_display("accept()");
				break;
			}
			else
			{
				// Ŭ���̾�Ʈ ���� ���
				char addr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &clientaddr4.sin_addr, addr, sizeof(addr));
				printf("[TCP/IPv4 ����] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n", addr, ntohs(clientaddr4.sin_port));
				
				// ���� ���� �߰� : ���� �� ���� ����
				if (!AddSocketInfo(client_sock, false, false))
					closesocket(client_sock);
			}
		}
		if (FD_ISSET(listen_sock6, &rset))
		{
			addrlen = sizeof(clientaddr6);
			client_sock = accept(listen_sock6, (struct sockaddr*)&clientaddr6, &addrlen);
			if (INVALID_SOCKET == client_sock)
			{
				err_display("accept()6");
				break;
			}
			else
			{
				// Ŭ���̾�Ʈ ���� ���
				char addr[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET, &clientaddr6.sin6_addr, addr, sizeof(addr));
				printf("[TCP/IPv4 ����] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n", addr, ntohs(clientaddr6.sin6_port));

				// ���� ���� �߰� : ���� �� ���� ����
				if (!AddSocketInfo(client_sock, true, false))
					closesocket(client_sock);
			}
		}

		// ���� �� �˻�(2) : ������ ���
		for (int i = 0; i < nTotalSockets; ++i)
		{
			SOCKETINFO* ptr = SocketInfoArray[i];
			if (FD_ISSET(ptr->sock, &rset))
			{
				// ������ ����
				retval = recv(ptr->sock, ptr->buf + ptr->recvbytes, BUFSIZE - ptr->recvbytes, 0);
				if (0 == retval || SOCKET_ERROR == retval)
				{
					RemoveSocketInfo(i);
					continue;
				}

				// ���� ����Ʈ �� ����
				ptr->recvbytes += retval;
				if (ptr->recvbytes == BUFSIZE)
				{
					// ���� ����Ʈ �� ����
					ptr->recvbytes = 0;
					
					// ���� ������ ��� Ŭ���̾�Ʈ�� ������ ����
					for (int j = 0; j < nTotalSockets; ++j)
					{
						SOCKETINFO* sender = SocketInfoArray[j];
						retval = send(sender->sock, ptr->buf, BUFSIZE, 0);
						if (SOCKET_ERROR == retval)
						{
							err_display("send()");
							RemoveSocketInfo(j);
							--j;
							continue;
						}
					}
				}
			}
		}
	}

	closesocket(listen_sock4);
	closesocket(listen_sock6);
	closesocket(listen_sock_UDP);
	closesocket(listen_sock_UDP6);
	WSACleanup();
	return 0;
}

bool AddSocketInfo(SOCKET sock, bool isIPv6, bool isUDP)
{
	if (nTotalSockets >= FD_SETSIZE)
	{
		printf("[����] ���� ������ �߰��� �� �����ϴ�!\n");
		return false;
	}

	SOCKETINFO* ptr = new SOCKETINFO;
	if (NULL == ptr)
	{
		printf("[����] �޸𸮰� �����մϴ�!\n");
		return false;
	}

	ptr->sock = sock;
	ptr->isIPv6 = isIPv6;
	ptr->isUDP = isUDP;
	ptr->recvbytes = 0;
	SocketInfoArray[nTotalSockets++] = ptr;
	return true;
}

void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO* ptr = SocketInfoArray[nIndex];
	if (ptr->isIPv6)
	{
		// Ŭ���̾�Ʈ ���� ���
		struct sockaddr_in6 clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (struct sockaddr*)&clientaddr, &addrlen);

		// Ŭ���̾�Ʈ ���� ���
		char addr[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &clientaddr.sin6_addr, addr, sizeof(addr));
		printf("[TCP/IPv6 ����] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n", addr, ntohs(clientaddr.sin6_port));
	}
	else
	{
		// Ŭ���̾�Ʈ ���� ���
		struct sockaddr_in clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (struct sockaddr*)&clientaddr, &addrlen);

		// Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP/IPv4 ����] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d\n", addr, ntohs(clientaddr.sin_port));
	}
	
	// ���� �ݱ�
	closesocket(ptr->sock);
	delete ptr;

	// ���� ������ ���� ������ �ε����� ������ �ε����� �ƴ� ��� ������ �ε����� ���� ������ ������ �ε����� �ڸ��� �Ű��ش�.
	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];
	
	--nTotalSockets;
}