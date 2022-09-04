#include <Windows.h>
#include <stdio.h>

#define MAXCNT 100000000

int count;
CRITICAL_SECTION cs;

DWORD WINAPI MyThread1(LPVOID arg)
{
	for (int i = 0; i < MAXCNT; ++i)
	{
		EnterCriticalSection(&cs);
		count += 2;
		LeaveCriticalSection(&cs);
	}

	return 0;
}

DWORD WINAPI MyThread2(LPVOID arg)
{
	for (int i = 0; i < MAXCNT; ++i)
	{
		EnterCriticalSection(&cs);
		count -= 2;
		LeaveCriticalSection(&cs);
	}
	
	return 0;
}

int main(int argc, char* argv[])
{
	// 임계 영역 초기화
	InitializeCriticalSection(&cs);

	// 스레드 두 개 생성
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, MyThread1, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, MyThread2, NULL, 0, NULL);
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	
	// 임계영역 삭제
	DeleteCriticalSection(&cs);

	printf("count = %d\n", count);
	return 0;
}