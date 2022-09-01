#include "Common.h"

#define TESTNAME "www.google.com"

// 도메인 이름으로 IPv4 주소 찾는 메소드
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

// IPv4 주소로 도메인 찾는 메소드
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

	printf("도메인 이름(변환 전) = %s\n", TESTNAME);

	// 도메인 이름 -> IP 주소
	struct in_addr addr;
	if (GetIPAddr(TESTNAME, &addr)) 
	{
		// 성공 시 결과 출력
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &addr, str, sizeof(str));
		printf("IP 주소(변환 후) = %s\n", str);

		char name[256];
		if (GetDomainName(addr, name, sizeof(name)))
		{
			// 성공 시 결과 출력
			printf("도메인 이름(다시 변환 후) = %s\n", name);
		}
	}

	closesocket(sock);
	
	WSACleanup();
	return 0;
}