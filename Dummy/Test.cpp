#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h> // 윈속2 확장 헤더
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

char* addrchar = (char*)"185.199.110.153";
#define DOMAIN "yozura.github.io"

bool GetIPAddr(const char* name, struct in_addr* addr)
{
	struct hostent* ptr = gethostbyname(name);
	if (NULL == ptr)
	{
		printf("Error gethostbyname()");
		return false;
	}
	if (AF_INET != ptr->h_addrtype)
		return false;

	memcpy(addr, ptr->h_addr, ptr->h_length);
	return true;
}

bool GetDomainName(struct in_addr addr, char* name, int namelen)
{
	struct hostent* ptr = gethostbyaddr((const char*)&addr, sizeof(addr), AF_INET);
	if (NULL == ptr)
	{
		printf("Error gethostbyaddr()");
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

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sock)
	{
		printf("Error socket()");
		return 1;
	}

	struct in_addr addr;
	if (GetIPAddr(DOMAIN, &addr))
	{
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &addr, str, sizeof(str));
		printf("IP 주소 = %s\n", str);

		char name[256];
		if (GetDomainName(addr, name, sizeof(name)))
		{
			printf("도메인 이름 = %s\n", name);
		}
	}

	int test = 0;
	int* testing = &test;
	printf("%d, %p\n", *testing, testing);
	*testing++ = 3;
	printf("%d\n", test);
	printf("%d, %p\n", *testing, testing);
	int tet = *testing++;
	printf("%d, %p", tet, testing);

	closesocket(sock);

	WSACleanup();
	return 0;
}