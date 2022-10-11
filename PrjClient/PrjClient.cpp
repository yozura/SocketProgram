#include "PrjClient.h"
#include "resource.h"

HINSTANCE		g_hInst;
LPCTSTR			g_lpszClass = TEXT("PrjClient");

HWND			g_hBtnSendFile;
HWND			g_hBtnSendMsg;
HWND			g_hEditStatus;
HWND			g_hBtnErasePic;
HWND			g_hDrawWnd;

SOCKET			g_sock;
HANDLE			g_hReadEvent;
HANDLE			g_hWriteEvent;
HANDLE			g_hClientThread;
char			g_ipaddr[64];
int				g_port;
volatile bool	g_isIPv6;
volatile bool	g_isUDP;
volatile bool	g_bCommStarted;

CHAT_MSG		g_chatMsg;
DRAWLINE_MSG	g_drawLineMsg;
int				g_drawColor;
ERASEPIC_MSG	g_erasePicMsg;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	// 이벤트 생성
	g_hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (NULL == g_hReadEvent) return 1;
	g_hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == g_hWriteEvent) return 1;

	// 전역 변수 초기화
	g_chatMsg.type = TYPE_CHAT;
	g_drawLineMsg.type = TYPE_DRAWLINE;
	g_drawLineMsg.color = RGB(255, 0, 0);
	g_erasePicMsg.type = TYPE_ERASEPIC;
	
	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	CloseHandle(g_hReadEvent);
	CloseHandle(g_hWriteEvent);

	WSACleanup();
	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hChkIsIPv6;
	static HWND hChkIsUDP;
	static HWND hStaticDummy;
	static HWND hEditIPaddr;
	static HWND hEditPort;
	static HWND hEditMsg;
	static HWND hEditStatus;
	static HWND hBtnErasePic;
	static HWND hBtnConnect;
	static HWND hBtnSendFile;
	static HWND hBtnSendMsg;
	static HWND hColorRed;
	static HWND hColorGreen;
	static HWND hColorBlue;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		// 컨트롤 핸들 얻기
		hChkIsIPv6 = GetDlgItem(hDlg, IDC_ISIPV6);
		hChkIsUDP = GetDlgItem(hDlg, IDC_ISUDP);
		hStaticDummy = GetDlgItem(hDlg, IDC_DUMMY);
		hEditIPaddr = GetDlgItem(hDlg, IDC_IPADDR);
		hEditPort = GetDlgItem(hDlg, IDC_PORT);
		hEditMsg = GetDlgItem(hDlg, IDC_MSG);
		hEditStatus = GetDlgItem(hDlg, IDC_STATUS);
		hBtnErasePic = GetDlgItem(hDlg, IDC_ERASEPIC);
		hBtnConnect = GetDlgItem(hDlg, IDC_CONNECT);
		hBtnSendFile = GetDlgItem(hDlg, IDC_SENDFILE);
		hBtnSendMsg = GetDlgItem(hDlg, IDC_SENDMSG);
		hColorRed = GetDlgItem(hDlg, IDC_COLORRED);
		hColorGreen = GetDlgItem(hDlg, IDC_COLORGREEN);
		hColorBlue = GetDlgItem(hDlg, IDC_COLORBLUE);

		g_hEditStatus = hEditStatus;
		g_hBtnErasePic = hBtnErasePic;
		g_hBtnSendFile = hBtnSendFile;
		g_hBtnSendMsg = hBtnSendMsg;

		SetDlgItemText(hDlg, IDC_IPADDR, SERVERIP4);
		SetDlgItemInt(hDlg, IDC_PORT, SERVERPORT, FALSE);
		EnableWindow(g_hBtnSendFile, FALSE);
		EnableWindow(g_hBtnSendMsg, FALSE);
		EnableWindow(g_hBtnErasePic, FALSE);
		SendMessage(hEditMsg, EM_SETLIMITTEXT, SIZE_DAT / 2, 0);
		SendMessage(hColorRed, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hColorGreen, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(hColorBlue, BM_SETCHECK, BST_UNCHECKED, 0);

		WNDCLASS wndclass;
		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = ChildWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = g_hInst;
		wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = _T("MyWndClass");
		if (!RegisterClass(&wndclass)) exit(1);

		RECT rect;
		GetWindowRect(hStaticDummy, &rect);
		POINT pt; pt.x = rect.left; pt.y = rect.top;
		ScreenToClient(hDlg, &pt);
		g_hDrawWnd = CreateWindow(
			_T("MyWndClass"), _T(""), WS_CHILD,
			pt.x, pt.y, rect.right - rect.left, rect.bottom - rect.top,
			hDlg, (HMENU)NULL, g_hInst, NULL);
		if (NULL == g_hDrawWnd) exit(1);
		ShowWindow(g_hDrawWnd, SW_SHOW);
		UpdateWindow(g_hDrawWnd);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ISIPV6:
			g_isIPv6 = SendMessage(hChkIsIPv6, BM_GETCHECK, 0, 0);
			if (g_isIPv6) SetDlgItemText(hDlg, IDC_IPADDR, SERVERIP6);
			else SetDlgItemText(hDlg, IDC_IPADDR, SERVERIP4);
			return TRUE;
		case IDC_CONNECT:
			GetDlgItemTextA(hDlg, IDC_IPADDR, g_ipaddr, sizeof(g_ipaddr));
			g_port = GetDlgItemInt(hDlg, IDC_PORT, NULL, TRUE);
			g_isIPv6 = SendMessage(hChkIsIPv6, BM_GETCHECK, 0, 0);
			g_isUDP = SendMessage(hChkIsUDP, BM_GETCHECK, 0, 0);

			// 소켓 통신 시작
			g_hClientThread = CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
			if (NULL == g_hClientThread) exit(0);

			// 서버 접속 대기
			while (!g_bCommStarted);

			EnableWindow(hChkIsIPv6, FALSE);
			EnableWindow(hChkIsUDP, FALSE);
			EnableWindow(hEditIPaddr, FALSE);
			EnableWindow(hEditPort, FALSE);
			EnableWindow(hBtnConnect, FALSE);
			EnableWindow(g_hBtnSendFile, FALSE);
			EnableWindow(g_hBtnSendMsg, TRUE);
			EnableWindow(g_hBtnErasePic, TRUE);
			SetFocus(hEditMsg);
			return TRUE;
		case IDC_SENDFILE:
			MessageBox(NULL, _T("아직 구현하지 않았습니다."), _T("알림"), MB_ICONERROR);
			// 1. GetOpenFileName() API를 이용해 파일 열기 대화상자를 열고
			// 2. 전송할 파일을 선택하고 파일을 읽어서 서버에 전송한다.
			return TRUE;
		case IDC_SENDMSG:
			// 이전에 얻은 채팅 메시지 읽기 완료를 기다림
			WaitForSingleObject(g_hReadEvent, INFINITE);
			// 새로운 채팅 메시지를 얻고 쓰기 완료를 알림
			GetDlgItemTextA(hDlg, IDC_MSG, g_chatMsg.msg, SIZE_DAT);
			SetEvent(g_hWriteEvent);
			// 입력된 텍스트 전체를 선택 표시
			SendMessage(hEditMsg, EM_SETSEL, 0, -1);
			return TRUE;
		case IDC_COLORRED:
			g_drawLineMsg.color = RGB(255, 0, 0);
			return TRUE;
		case IDC_COLORGREEN:
			g_drawLineMsg.color = RGB(0, 255, 0);
			return TRUE;
		case IDC_COLORBLUE:
			g_drawLineMsg.color = RGB(0, 0, 255);
			return TRUE;
		case IDC_ERASEPIC:
			send(g_sock, (char*)&g_erasePicMsg, SIZE_TOT, 0);
			return TRUE;
		case IDCANCEL:
			closesocket(g_sock);
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}

	return FALSE;
}

LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HPEN hPen, hOldPen;
	PAINTSTRUCT ps;
	static int cx, cy;
	static HBITMAP hBitmap;
	static HDC hMemDC;
	static int x0, y0;
	static int x1, y1;
	static bool bDrawing;

	switch (uMsg)
	{
	case WM_SIZE:
		hdc = GetDC(hWnd);
		// 배경 비트맵과 메모리 DC 생성
		cx = LOWORD(lParam);
		cy = HIWORD(lParam);
		hBitmap = CreateCompatibleBitmap(hdc, cx, cy);
		hMemDC = CreateCompatibleDC(hdc);
		SelectObject(hMemDC, hBitmap);
		// 배경 비트맵 흰색으로 채움
		SelectObject(hMemDC, GetStockObject(WHITE_BRUSH));
		SelectObject(hMemDC, GetStockObject(WHITE_PEN));
		Rectangle(hMemDC, 0, 0, cx, cy);

		ReleaseDC(hWnd, hdc);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, cx, cy, hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_LBUTTONDOWN:
		x0 = LOWORD(lParam);
		y0 = HIWORD(lParam);
		bDrawing = true;
		return 0;
	case WM_MOUSEMOVE:
		if (bDrawing && g_bCommStarted)
		{
			x1 = LOWORD(lParam);
			y1 = HIWORD(lParam);

			g_drawLineMsg.x0 = x0;
			g_drawLineMsg.x1 = x1;
			g_drawLineMsg.y0 = y0;
			g_drawLineMsg.y1 = y1;
			send(g_sock, (char*)&g_drawLineMsg, SIZE_TOT, 0);

			x0 = x1;
			y0 = y1;
		}
		return 0;
	case WM_LBUTTONUP:
		bDrawing = false;
		return 0;
	case WM_DRAWLINE:
		hdc = GetDC(hWnd);
		hPen = CreatePen(PS_SOLID, 3, g_drawColor);
		hOldPen = (HPEN)SelectObject(hdc, hPen);
		// 윈도우 화면에 1차로 그리기
		MoveToEx(hdc, LOWORD(wParam), HIWORD(wParam), NULL);
		LineTo(hdc, LOWORD(lParam), HIWORD(lParam));
		SelectObject(hdc, hOldPen);

		// 배경 비트맵에 2차로 그리기
		hOldPen = (HPEN)SelectObject(hMemDC, hPen);
		MoveToEx(hMemDC, LOWORD(wParam), HIWORD(wParam), NULL);
		LineTo(hMemDC, LOWORD(lParam), HIWORD(lParam));

		DeleteObject(SelectObject(hMemDC, hOldPen));
		ReleaseDC(hWnd, hdc);
		return 0;
	case WM_ERASEPIC:
		SelectObject(hMemDC, GetStockObject(WHITE_BRUSH));
		SelectObject(hMemDC, GetStockObject(WHITE_PEN));
		Rectangle(hMemDC, 0, 0, cx, cy);
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_DESTROY:
		DeleteDC(hMemDC);
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	if (!g_isIPv6 && !g_isUDP) /* TCP / IPv4에 접속 */
	{
		g_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == g_sock) err_quit("socket()");

		struct sockaddr_in serveraddr;
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		inet_pton(AF_INET, g_ipaddr, &serveraddr.sin_addr);
		serveraddr.sin_port = htons(g_port);

		retval = connect(g_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		if (SOCKET_ERROR == retval) err_quit("connect()");
	}
	else if (g_isIPv6 && !g_isUDP)
	{
		g_sock = socket(AF_INET6, SOCK_STREAM, 0);
		if (INVALID_SOCKET == g_sock) err_quit("socket()");

		struct sockaddr_in6 serveraddr;
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin6_family = AF_INET6;
		inet_pton(AF_INET6, g_ipaddr, &serveraddr.sin6_addr);
		serveraddr.sin6_port = htons(g_port);

		retval = connect(g_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		if (SOCKET_ERROR == retval) err_quit("connect()");
	}
	else if (!g_isIPv6 && g_isUDP)
	{
		// 미구현
		MessageBox(NULL, _T("아직 구현하지 않았습니다."), _T("알림"), MB_ICONERROR);
		exit(1);
	}
	else if (g_isIPv6 && g_isUDP)
	{
		MessageBox(NULL, _T("아직 구현하지 않았습니다."), _T("알림"), MB_ICONERROR);
		exit(1);
	}
	MessageBox(NULL, _T("서버에 접속했습니다."), _T("알림"), MB_ICONINFORMATION);

	// 읽기 & 쓰기 스레드 생성
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	if (NULL == hThread[0] || NULL == hThread[1]) exit(1);
	g_bCommStarted = true;

	// 스레드 종료 대기 (둘 중 하나라도 종료할 때까지)
	retval = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	retval -= WAIT_OBJECT_0;
	if (0 == retval) TerminateThread(hThread[1], 1);
	else TerminateThread(hThread[0], 1);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	MessageBox(NULL, _T("연결이 끊겼습니다."), _T("알림"), MB_ICONERROR);
	EnableWindow(g_hBtnSendFile, FALSE);
	EnableWindow(g_hBtnSendMsg, FALSE);
	EnableWindow(g_hBtnErasePic, FALSE);
	g_bCommStarted = false;
	closesocket(g_sock);
	return 0;
}

DWORD WINAPI ReadThread(LPVOID arg)
{
	int retval;
	COMM_MSG comm_msg;
	CHAT_MSG* chat_msg;
	DRAWLINE_MSG* drawline_msg;
	ERASEPIC_MSG* erasepic_msg;

	while (true)
	{
		retval = recv(g_sock, (char*)&comm_msg, SIZE_TOT, MSG_WAITALL);
		if (0 == retval || SOCKET_ERROR == retval) break;
		if (comm_msg.type == TYPE_CHAT)
		{
			chat_msg = (CHAT_MSG*)&comm_msg;
			DisplayText("[받은 메시지] %s\r\n", chat_msg->msg);
		}
		else if (comm_msg.type == TYPE_DRAWLINE)
		{
			drawline_msg = (DRAWLINE_MSG*)&comm_msg;
			g_drawColor = drawline_msg->color;
			SendMessage(g_hDrawWnd, WM_DRAWLINE,
				MAKEWPARAM(drawline_msg->x0, drawline_msg->y0),
				MAKELPARAM(drawline_msg->x1, drawline_msg->y1));
		}
		else if (comm_msg.type == TYPE_ERASEPIC)
		{
			erasepic_msg = (ERASEPIC_MSG*)&comm_msg;
			SendMessage(g_hDrawWnd, WM_ERASEPIC, 0, 0);
		}
	}
	return 0;
}

DWORD WINAPI WriteThread(LPVOID arg)
{
	int retval;

	while (true)
	{
		WaitForSingleObject(g_hWriteEvent, INFINITE);
		if (strlen(g_chatMsg.msg) == 0)
		{
			EnableWindow(g_hBtnSendMsg, TRUE);
			SetEvent(g_hReadEvent);
			continue;
		}

		retval = send(g_sock, (char*)&g_chatMsg, SIZE_TOT, 0);
		if (SOCKET_ERROR == retval) break;

		EnableWindow(g_hBtnSendMsg, TRUE);
		SetEvent(g_hReadEvent);
	}
	
	return 0;
}

void DisplayText(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	char cbuf[1024];
	vsprintf(cbuf, fmt, arg);
	va_end(arg);

	int nLength = GetWindowTextLength(g_hEditStatus);
	SendMessage(g_hEditStatus, EM_SETSEL, nLength, nLength);
	SendMessageA(g_hEditStatus, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
}