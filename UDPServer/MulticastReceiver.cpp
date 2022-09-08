#include "Common.h"

#define MULTICASTIP	"235.7.8.9"
#define	LOCALPORT	9000
#define BUFSIZE		512

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == sock) err_quit("sock()");

	DWORD optval = 1;
	retval = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
	if (SOCKET_ERROR == retval) err_quit("setsockopt()");

	struct sockaddr_in localaddr;
	memset(&localaddr, 0, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(LOCALPORT);
	retval = bind(sock, (struct sockaddr*)&localaddr, sizeof(localaddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	// ��Ƽĳ��Ʈ �׷� ����
	struct ip_mreq mreq;
	inet_pton(AF_INET, MULTICASTIP, &mreq.imr_multiaddr);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
	if (SOCKET_ERROR == retval) err_quit("setsockopt()");

	struct sockaddr_in peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// ��Ƽĳ��Ʈ ������ �ޱ�
	while (1)
	{
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&peeraddr, &addrlen);
		if (SOCKET_ERROR == retval)
		{
			err_display("recvfrom()");
			break;
		}

		// ���� ������ ���
		buf[retval] = '\0';
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &peeraddr.sin_addr, addr, sizeof(addr));
		printf("[UDP/%s:%d] %s\n", addr, ntohs(peeraddr.sin_port), buf);
	}

	// ��Ƽĳ��Ʈ �׷� Ż��
	retval = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
	if (SOCKET_ERROR == retval) err_quit("setsockopt()");

	closesocket(sock);
	WSACleanup();
	return 0;
}
