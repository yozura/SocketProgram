#include "Common.h"
#include "resource.h"

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define BUFSIZE		512

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

// 에디트 컨트롤 출력 함수
void DisplayText(const char* fmt, ...);

// 소켓 함수 오류 출력
void DisplayError(const char* msg);

// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg);

SOCKET sock;
char buf[BUFSIZE + 1];
HANDLE hReadEvent, hWriteEvent;
HWND hSendButton;
HWND hEdit1, hEdit2;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);

	WSACleanup();
	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		hSendButton = GetDlgItem(hDlg, IDOK);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EnableWindow(hSendButton, FALSE);
			WaitForSingleObject(hReadEvent, INFINITE);
			GetDlgItemTextA(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			SetEvent(hWriteEvent);
			SetFocus(hEdit1);
			SendMessage(hEdit1, EM_SETSEL, 0, -1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			closesocket(sock);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

void DisplayText(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	char cBuf[BUFSIZE * 2];
	vsprintf(cBuf, fmt, arg); 
	va_end(arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessageA(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cBuf);
}

void DisplayError(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	DisplayText("[%s] %s\r\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sock) err_quit("socket()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("connect()");

	while (1)
	{
		WaitForSingleObject(hWriteEvent, INFINITE);

		if (strlen(buf) == 0)
		{
			EnableWindow(hSendButton, TRUE);
			SetEvent(hReadEvent);
			continue;
		}

		retval = send(sock, buf, (int)strlen(buf), 0);
		if (SOCKET_ERROR == retval)
		{
			DisplayError("send()");
			break;
		}

		DisplayText("[TCP 클라이언트] %d바이트를 보냈습니다.\r\n", retval);

		retval = recv(sock, buf, retval, MSG_WAITALL);
		if (SOCKET_ERROR == retval)
		{
			DisplayError("recv()");
			break;
		}
		else if (0 == retval)
			break;

		buf[retval] = '\0';
		DisplayText("[TCP 클라이언트] %d바이트를 받았습니다.\r\n", retval);
		DisplayText("[받은 데이터] %s\r\n", buf);

		EnableWindow(hSendButton, TRUE);
		SetEvent(hReadEvent);
	}

	return 0;
}
