#include <windows.h>
#include <stdio.h>

struct Point3D
{
	int x, y, z;
};

DWORD WINAPI MyPriority(LPVOID arg)
{
	while (1);
	return 0;
}

// 스레드 함수의 기본 형식
DWORD WINAPI MyThread(LPVOID arg)
{
	Sleep(1000);
	Point3D* pt = (Point3D*)arg;
	printf("Running MyThread() %d: %d, %d, %d\n", GetCurrentThreadId(), pt->x, pt->y, pt->z);
	return 0;
}

int main()
{
	// 우선 순위 값의 범위를 출력한다.
	printf("우선순위: %d ~ %d\n", THREAD_PRIORITY_IDLE, THREAD_PRIORITY_TIME_CRITICAL);

	// CPU 개수를 알아낸다.
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// CPU 개수만큼 스레드를 생성한다.
	for (int i = 0; i < (int)si.dwNumberOfProcessors; ++i)
	{
		// 스레드 생성
		HANDLE hThread = CreateThread(NULL, 0, MyPriority, NULL, 0, NULL);
		// 우선 순위 높게 설정
		SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
		CloseHandle(hThread);
	}

	// 우선 순위 낮게 설정
	SetThreadPriority(GetCurrentThread, THREAD_PRIORITY_BELOW_NORMAL);
	Sleep(1000);
	printf("Running main() %d\n", GetCurrentThreadId());
	return 0;
}