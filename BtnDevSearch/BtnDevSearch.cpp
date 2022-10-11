#include "Common.h"
#include <ws2bth.h>
#include <locale.h>

int main(int argc, char* argv[])
{
	int retval;

	// Unicode �ѱ��� ���
	setlocale(LC_ALL, "");

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ������� ��ġ �˻� �غ�
	DWORD qslen = sizeof(WSAQUERYSET);
	WSAQUERYSET* qs = (WSAQUERYSET*)malloc(qslen);
	memset(qs, 0, qslen);
	qs->dwSize = qslen;
	qs->dwNameSpace = NS_BTH;
	DWORD flags = LUP_CONTAINERS | LUP_FLUSHCACHE | LUP_RETURN_NAME | LUP_RETURN_ADDR;

	// ������� ��ġ �˻� ����
	HANDLE hLookup;
	retval = WSALookupServiceBegin(qs, flags, &hLookup);
	if (SOCKET_ERROR == retval) 
	{
		printf("[����] �߰ߵ� ������� ��ġ ����!\n");
		exit(1);
	}

	// �˻��� ��ġ ���� ���
	SOCKADDR_BTH* sa = NULL;
	bool done = false;
	while (!done)
	{
		retval = WSALookupServiceNext(hLookup, flags, &qslen, qs);
		if (NO_ERROR == retval)
		{
			// ���� ��ġ�� ���� ���� �ּ� ����ü ����
			sa = (SOCKADDR_BTH*)qs->lpcsaBuffer->RemoteAddr.lpSockaddr;

			// ������� ��ġ �ּҸ� ���ڿ��� ���
			TCHAR addr[40] = { 0, };
			DWORD addrlen = sizeof(addr);
			WSAAddressToString((struct sockaddr*)sa, sizeof(SOCKADDR_BTH), NULL, addr, &addrlen);
			_tprintf(_T("������� ��ġ �߰�!  %s - %s\n"), addr, qs->lpszServiceInstanceName);
		}
		else
		{
			if (WSAGetLastError() == WSAEFAULT) 
			{
				free(qs);
				qs = (WSAQUERYSET*)malloc(qslen);
			}
			else
			{
				done = true;
			}
		}
	}

	// ������� ��ġ �˻� ����
	WSALookupServiceEnd(hLookup);
	free(qs);

	WSACleanup();
	return 0;
}