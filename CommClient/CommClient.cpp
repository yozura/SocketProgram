#include "Common.h"

#define	PORTNAME	_T("\\\\.\\COM1")
#define BUFSIZE		512

int main(int argc, char* argv[])
{
	int retval;

	// ���� ��Ʈ ����
	HANDLE hComm = CreateFile(PORTNAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hComm) err_quit("CreateFile()");

	// ���� ��Ʈ ������ ���
	DCB dcb;
	if (!GetCommState(hComm, &dcb)) err_quit("GetCommState()");

	// ���� ��Ʈ ������ ����
	dcb.BaudRate = CBR_57600;
	dcb.ByteSize = 8;
	dcb.fParity = FALSE;
	dcb.StopBits = ONESTOPBIT;
	if (!SetCommState(hComm, &dcb)) err_quit("SetCommState()");

	// �б�� ���� Ÿ�Ӿƿ� ����
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	if (!SetCommTimeouts(hComm, &timeouts)) err_quit("SetCommTimeouts()");

	char buf[BUFSIZE + 1];
	int len;
	DWORD BytesRead, BytesWritten;

	while (1)
	{
		memset(buf, 0, sizeof(buf));
		printf("\n[���� ������] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// '\n' ���� ����
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n') buf[len - 1] = '\0';
		if (strlen(buf) == 0) break;

		// ������ ������
		retval = WriteFile(hComm, buf, BUFSIZE, &BytesWritten, NULL);
		if (0 == retval) err_quit("WriteFile()");
		printf("[Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", BytesWritten);
		
		// ������ �ޱ�
		retval = ReadFile(hComm, buf, BUFSIZE, &BytesRead, NULL);
		if (0 == retval) err_quit("ReadFile()");
		
		// ���� ������ ���
		buf[BytesRead] = '\0';
		printf("[Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", BytesRead);
		printf("[���� ������] %s\n", buf);
	}

	CloseHandle(hComm);
	return 0;
}