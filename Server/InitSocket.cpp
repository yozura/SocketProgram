#include "Common.h"

int f(int x)
{
	if (x >= 0)
	{
		WSASetLastError(0);
		return 0;
	}

	WSASetLastError(WSAEINVAL);
	return SOCKET_ERROR;
}

int createSocket()
{
	// 1. ���� �ʱ�ȭ
	WSADATA wsa;
	if (!WSAStartup(MAKEWORD(2, 2), &wsa))
		return 1;

	// 2. ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) err_quit("sock()");
	
	// ���� �ݱ�
	closesocket(sock);
	
	// ���� ����
	WSACleanup();
	return 0;
}


int main()
{
	// 1. ���� �ʱ�ȭ
	// ���� ������ ��û�Ͽ� ���� ���̺귯���� �ʱ�ȭ�Ѵ�.
	// �ʱ�ȭ ���� �� ���̺귯�� ȣ�� ����
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		printf("[���] ���� �ʱ�ȭ ����\n");
		return 1;
	}
	printf("[�˸�] ���� �ʱ�ȭ ����\n");
	printf("%d, %d, %s, %s\n", wsa.wVersion, wsa.wHighVersion, wsa.szDescription, wsa.szSystemStatus);

	u_short s1 = 0x1234;
	u_long l1 = 0x12345678;
	u_short s2;
	u_long l2;

	// s1�� s2�� ��Ʈ��ũ ����Ʈ ����
	// l1�� l2�� ��Ʈ��ũ ����Ʈ ����
	printf("[Host Byte -> Network Byte]\n");
	printf("%#x -> %#x\n", s1, s2 = htons(s1));
	printf("%#x -> %#x\n", l1, l2 = htonl(l1));

	printf("[Network Byte -> Host Byte]\n");
	printf("%#x -> %#x\n", s2, ntohs(s2));
	printf("%#x -> %#x\n", l2, ntohl(l2));

	// �߸��� ��� ��
	printf("\n[�߸��� ��� ��]\n");
	printf("%#x -> %#x\n\n", s1, htonl(s1));
	
	/* IPv4 ��ȯ ���� */
	// ���� IPv4 �ּ� ���
	const char* ipv4test = "147.46.114.70";
	printf("IPv4 �ּ�(��ȯ ��) = %s\n", ipv4test);

	// inet_pton() �Լ� ����
	struct in_addr ipv4num;
	inet_pton(AF_INET, ipv4test, &ipv4num);
	printf("IPv4 �ּ�(��ȯ ��) = %#x\n", ipv4num.s_addr);

	char ipv4str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ipv4num, ipv4str, sizeof(ipv4str));
	printf("IPv4 �ּ�(�ٽ� ��ȯ ��) = %s\n", ipv4str);
	
	/* IPv6 ��ȯ ���� */
	// ���� IPv6 �ּ� ���
	const char* ipv6test = "2001:0230:abcd:ffab:0023:eb00:ffff:1111";
	printf("IPv6 �ּ�(��ȯ ��) = %s\n", ipv6test);

	// inet_pton() �Լ� ����
	struct in6_addr ipv6num;
	inet_pton(AF_INET6, ipv6test, &ipv6num);
	printf("IPv6 �ּ�(��ȯ ��) = 0x");
	for (int i = 0; i < 16; ++i)
		printf("%02x", ipv6num.s6_addr[i]);
	printf("\n");

	// inet_ntop() �Լ� ����
	char ipv6str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &ipv6num, ipv6str, sizeof(ipv6str));
	printf("IPv6 �ּ�(�ٽ� ��ȯ ��) = %s\n", ipv6str);

	// 2. ���� ����
	// ���� �μ��� �ּ� ü��(Address Family), ���� Ÿ��(Socket Type), ��������(Protocol)
	// �������ݿ� ���� �������� �����ؾ��ϸ� ���� ����ϴ� TCP�� UDP�� �̷���.
	// TCP : socket(AF_INET, SOCK_STREAM, 0);
	// UDP : socket(AF_INET, SOCK_DGRAM, 0);
	
	
	// WSASocket() �Լ��� ������ API������ �����ϴ� ������ ����� ���� �����Դϴ�.
	// SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[�˸�] ���� ���� ����\n");

	// ���� �ݱ�
	// ������ �ݰ� ���ҽ� ��ȯ
	closesocket(sock);

	// ���� ����
	// ���� ����� �������� �ü���� �˷��� ���ҽ� ��ȯ
	WSACleanup();
	return 0;
}