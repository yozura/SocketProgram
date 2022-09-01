#include "Common.h"

int f(int x)
{
	if (x >= 0)
	{
		WSASetLastError(0);
		return 0;
	}

	WSASetLastError(WSAEINVAL);
	return SOCKET_ERROR;
}

int createSocket()
{
	// 1. 윈속 초기화
	WSADATA wsa;
	if (!WSAStartup(MAKEWORD(2, 2), &wsa))
		return 1;

	// 2. 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) err_quit("sock()");
	
	// 소켓 닫기
	closesocket(sock);
	
	// 윈속 종료
	WSACleanup();
	return 0;
}


int main()
{
	// 1. 윈속 초기화
	// 윈속 버전을 요청하여 윈속 라이브러리를 초기화한다.
	// 초기화 실패 시 라이브러리 호출 실패
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		printf("[경고] 윈속 초기화 실패\n");
		return 1;
	}
	printf("[알림] 윈속 초기화 성공\n");
	printf("%d, %d, %s, %s\n", wsa.wVersion, wsa.wHighVersion, wsa.szDescription, wsa.szSystemStatus);

	u_short s1 = 0x1234;
	u_long l1 = 0x12345678;
	u_short s2;
	u_long l2;

	// s1을 s2로 네트워크 바이트 정렬
	// l1을 l2로 네트워크 바이트 정렬
	printf("[Host Byte -> Network Byte]\n");
	printf("%#x -> %#x\n", s1, s2 = htons(s1));
	printf("%#x -> %#x\n", l1, l2 = htonl(l1));

	printf("[Network Byte -> Host Byte]\n");
	printf("%#x -> %#x\n", s2, ntohs(s2));
	printf("%#x -> %#x\n", l2, ntohl(l2));

	// 잘못된 사용 예
	printf("\n[잘못된 사용 예]\n");
	printf("%#x -> %#x\n\n", s1, htonl(s1));
	
	/* IPv4 변환 연습 */
	// 원래 IPv4 주소 출력
	const char* ipv4test = "147.46.114.70";
	printf("IPv4 주소(변환 전) = %s\n", ipv4test);

	// inet_pton() 함수 연습
	struct in_addr ipv4num;
	inet_pton(AF_INET, ipv4test, &ipv4num);
	printf("IPv4 주소(변환 후) = %#x\n", ipv4num.s_addr);

	char ipv4str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ipv4num, ipv4str, sizeof(ipv4str));
	printf("IPv4 주소(다시 변환 후) = %s\n", ipv4str);
	
	/* IPv6 변환 연습 */
	// 원래 IPv6 주소 출력
	const char* ipv6test = "2001:0230:abcd:ffab:0023:eb00:ffff:1111";
	printf("IPv6 주소(변환 전) = %s\n", ipv6test);

	// inet_pton() 함수 연습
	struct in6_addr ipv6num;
	inet_pton(AF_INET6, ipv6test, &ipv6num);
	printf("IPv6 주소(변환 후) = 0x");
	for (int i = 0; i < 16; ++i)
		printf("%02x", ipv6num.s6_addr[i]);
	printf("\n");

	// inet_ntop() 함수 연습
	char ipv6str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &ipv6num, ipv6str, sizeof(ipv6str));
	printf("IPv6 주소(다시 변환 후) = %s\n", ipv6str);

	// 2. 소켓 생성
	// 전달 인수는 주소 체계(Address Family), 소켓 타입(Socket Type), 프로토콜(Protocol)
	// 프로토콜에 따라 설정값을 변경해야하며 보통 사용하는 TCP와 UDP는 이렇다.
	// TCP : socket(AF_INET, SOCK_STREAM, 0);
	// UDP : socket(AF_INET, SOCK_DGRAM, 0);
	
	
	// WSASocket() 함수는 윈도우 API에서만 지원하는 강력한 기능을 가진 소켓입니다.
	// SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[알림] 소켓 생성 성공\n");

	// 소켓 닫기
	// 소켓을 닫고 리소스 반환
	closesocket(sock);

	// 윈속 종료
	// 윈속 사용을 중지함을 운영체제에 알려서 리소스 반환
	WSACleanup();
	return 0;
}