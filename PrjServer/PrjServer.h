#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define	_WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32")

void err_quit(const char* msg)
{
	LPVOID lpszbuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpszbuf, 0, NULL);
	MessageBoxA(NULL, (LPCSTR)lpszbuf, msg, MB_OK);
	LocalFree(lpszbuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpszbuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpszbuf, 0, NULL);
	MessageBoxA(NULL, (LPCSTR)lpszbuf, msg, MB_OK);
	LocalFree(lpszbuf);
}