#include "Common.h"

#define TESTNAME "www.google.com"

// ������ �̸����� IPv4 �ּ� ã�� �޼ҵ�
bool GetIPAddr(const char* name, struct in_addr* addr)
{
	struct hostent* ptr = gethostbyname(name);
	if (NULL == ptr)
	{
		err_display("gethostbyname()");
		return false;
	}
	if (AF_INET != ptr->h_addrtype)
		return false;

	memcpy(addr, ptr->h_addr, ptr->h_length);
	return true;
}

// IPv4 �ּҷ� ������ ã�� �޼ҵ�
bool GetDomainName(struct in_addr addr, char* name, int namelen)
{
	struct hostent* ptr = gethostbyaddr((const char*)&addr, sizeof(addr), AF_INET);
	if (NULL == ptr)
	{
		err_display("gethostbyname()");
		return false;
	}
	if (AF_INET != ptr->h_addrtype)
		return false;

	strncpy(name, ptr->h_name, namelen);
	return true;
}

int method()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sock) err_quit("sock()");

	printf("������ �̸�(��ȯ ��) = %s\n", TESTNAME);

	// ������ �̸� -> IP �ּ�
	struct in_addr addr;
	if (GetIPAddr(TESTNAME, &addr)) 
	{
		// ���� �� ��� ���
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &addr, str, sizeof(str));
		printf("IP �ּ�(��ȯ ��) = %s\n", str);

		char name[256];
		if (GetDomainName(addr, name, sizeof(name)))
		{
			// ���� �� ��� ���
			printf("������ �̸�(�ٽ� ��ȯ ��) = %s\n", name);
		}
	}

	closesocket(sock);
	
	WSACleanup();
	return 0;
}