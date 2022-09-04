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

// ������ �Լ��� �⺻ ����
DWORD WINAPI MyThread(LPVOID arg)
{
	Sleep(1000);
	Point3D* pt = (Point3D*)arg;
	printf("Running MyThread() %d: %d, %d, %d\n", GetCurrentThreadId(), pt->x, pt->y, pt->z);
	return 0;
}

int main()
{
	// �켱 ���� ���� ������ ����Ѵ�.
	printf("�켱����: %d ~ %d\n", THREAD_PRIORITY_IDLE, THREAD_PRIORITY_TIME_CRITICAL);

	// CPU ������ �˾Ƴ���.
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// CPU ������ŭ �����带 �����Ѵ�.
	for (int i = 0; i < (int)si.dwNumberOfProcessors; ++i)
	{
		// ������ ����
		HANDLE hThread = CreateThread(NULL, 0, MyPriority, NULL, 0, NULL);
		// �켱 ���� ���� ����
		SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
		CloseHandle(hThread);
	}

	// �켱 ���� ���� ����
	SetThreadPriority(GetCurrentThread, THREAD_PRIORITY_BELOW_NORMAL);
	Sleep(1000);
	printf("Running main() %d\n", GetCurrentThreadId());
	return 0;
}