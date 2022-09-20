#include "Common.h"
#include <ws2bth.h>
#include <locale.h>

int main(int argc, char* argv[])
{
	int retval;

	// Unicode 한국어 출력
	setlocale(LC_ALL, "");

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 블루투스 장치 검색 준비
	DWORD qslen = sizeof(WSAQUERYSET);
	WSAQUERYSET* qs = (WSAQUERYSET*)malloc(qslen);
	memset(qs, 0, qslen);
	qs->dwSize = qslen;
	qs->dwNameSpace = NS_BTH;
	DWORD flags = LUP_CONTAINERS;
	flags |= (LUP_FLUSHCACHE | LUP_RETURN_NAME | LUP_RETURN_ADDR);

	// 블루투스 장치 검색 시작
	HANDLE hLookup;
	retval = WSALookupServiceBegin(qs, flags, &hLookup);


	WSACleanup();
	return 0;
}