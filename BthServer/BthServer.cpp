#include "Common.h"
#include <ws2bth.h>
#include <initguid.h>

#define BUFSIZE	512

DEFINE_GUID(BthServer_Service, 0x4672de25, 0x588d, 0x48af, 0x80, 0x73, 0x5f, 0x2b, 0x7b, 0x0, 0x60, 0x1f);

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listen_sock = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (INVALID_SOCKET == listen_sock) err_quit("socket()");

	SOCKADDR_BTH serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.addressFamily = AF_BTH;
	serveraddr.btAddr = 0;
	serveraddr.port = BT_PORT_ANY;
	
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("bind()");

	// ���� ��Ʈ ��ȣ ��� (�ɼ�)
	int addrlen = sizeof(serveraddr);
	retval = getsockname(listen_sock, (struct sockaddr*)&serveraddr, &addrlen);
	if (SOCKET_ERROR == retval) err_quit("bind()");
	printf("[������� ����] ��� ��Ʈ ��ȣ : %d\n", serveraddr.port);

	// ���� ���� ��� (�ʼ�)
	CSADDR_INFO addrInfo;
	addrInfo.LocalAddr.lpSockaddr = (struct sockaddr*)&serveraddr;
	addrInfo.LocalAddr.iSockaddrLength = sizeof(serveraddr);
	addrInfo.RemoteAddr.lpSockaddr = (struct sockaddr*)&serveraddr;
	addrInfo.RemoteAddr.iSockaddrLength = sizeof(serveraddr);
	addrInfo.iSocketType = SOCK_STREAM;
	addrInfo.iProtocol = BTHPROTO_RFCOMM;

	WSAQUERYSET qset;
	memset(&qset, 0, sizeof(qset));
	qset.dwSize = sizeof(qset);
	qset.lpszServiceInstanceName = (LPTSTR)_T("Bluetooth Server Test Service");
	qset.lpServiceClassId = (GUID*)&BthServer_Service;
	qset.dwNameSpace = NS_BTH;
	qset.dwNumberOfCsAddrs = 1;
	qset.lpcsaBuffer = &addrInfo;
	retval = WSASetService(&qset, RNRSERVICE_REGISTER, 0);
	if (SOCKET_ERROR == retval) err_quit("WSASetService()");

	retval = listen(listen_sock, 1);
	if (SOCKET_ERROR == retval) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_BTH clientaddr;
	char buf[BUFSIZE + 1];

	while (true)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&client_sock, &addrlen);
		if (INVALID_SOCKET == client_sock)
		{
			err_display("accept()");
			break;
		}

		printf("\n[������� ����] Ŭ���̾�Ʈ ����!\n");

		// Ŭ���̾�Ʈ�� ������ ���
		while (true)
		{
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (SOCKET_ERROR == retval)
			{
				err_display("recv()");
				break;
			}
			else if (0 == retval)
				break;

			buf[retval] = '\0';
			printf("[������� ����] %s\n", buf);
		}

		closesocket(client_sock);
		printf("[������� ����] Ŭ���̾�Ʈ ����!\n");
	}

	closesocket(listen_sock);
	WSACleanup();
	return 0;
}