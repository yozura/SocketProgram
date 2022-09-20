#include "Common.h"

#define SERVERPORT	9000
#define BUFSIZE		512

struct SOCKETINFO
{
	OVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
};

DWORD WINAPI WorkerThread(LPVOID arg);

int main(int argc, char* argv[])
{
	int retval;
	
	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;

	// 입출력 완료 포트 생성
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == hcp) return 1;

	// 컴퓨터 정보 받아오기
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// CPU * 2 개수의 작업자 쓰레드 생성
	HANDLE hThread;
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; ++i)
	{
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		if (NULL == hThread) return 1;
		CloseHandle(hThread);
	}

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_sock) err_quit("sock()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("listen()");

	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	DWORD recvbytes, flags;

	while (1)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (INVALID_SOCKET == client_sock)
		{
			err_display("accept()");
			break;
		}

		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP 서버] 클라이언트 접속 : IP 주소 = %s, 포트 번호 = %d", addr, ntohs(clientaddr.sin_port));

		// 소켓과 입출력 완료 포트 연결
		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);
		
		// 소켓 정보 구조체 할당
		SOCKETINFO* ptr = new SOCKETINFO;
		if (NULL == ptr) break;
		memset(&ptr->overlapped, 0, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		// 비동기 입출력 시작
		flags = 0;
		retval = WSARecv(client_sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
		if (SOCKET_ERROR == retval)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
				err_display("WSARecv()");
			continue;
		}
	}

	WSACleanup();
	return 0;
}

DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;
	HANDLE hcp = (HANDLE)arg;
	
	while (1)
	{
		// 비동기 입출력 완료 기다리기
		DWORD cbTransferred;
		SOCKET client_sock;
		SOCKETINFO* ptr;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (PULONG_PTR)&client_sock, (LPOVERLAPPED*)&ptr, INFINITE);

		// 클라이언트 정보 얻기
		struct sockaddr_in clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (struct sockaddr*)&clientaddr, &addrlen);
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

		// 비동기 입출력 결과 확인
		if (0 == retval || 0 == cbTransferred)
		{
			printf("[TCP 서버] 클라이언트 종료 : IP 주소 = %s, 포트 번호 = %d", addr, ntohs(clientaddr.sin_port));
			closesocket(ptr->sock);
			delete ptr;
			continue;
		}

		// 데이터 전송량 갱신
		if (0 == ptr->recvbytes)
		{
			ptr->recvbytes = cbTransferred;
			ptr->sendbytes = 0;

			// 받은 데이터 출력
			ptr->buf[ptr->recvbytes] = '\0';
			printf("\n[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), ptr->buf);
		}
		else ptr->sendbytes += cbTransferred;

		if (ptr->recvbytes > ptr->sendbytes)
		{
			// 데이터 보내기
			memset(&ptr->overlapped, 0, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf + ptr->sendbytes;
			ptr->wsabuf.len = ptr->recvbytes - ptr->sendbytes;

			DWORD sendbytes;
			retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendbytes, 0, &ptr->overlapped, NULL);
			if (SOCKET_ERROR == retval)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
					err_display("WSASend()");
				continue;
			}
		}
		else
		{
			ptr->recvbytes = 0;
			
			// 데이터 받기
			memset(&ptr->overlapped, 0, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUFSIZE;

			DWORD recvbytes;
			DWORD flags = 0;
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
			if (SOCKET_ERROR == retval)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
					err_display("WSARecv()");
				continue;
			}
		}
	}

	return 0;
}