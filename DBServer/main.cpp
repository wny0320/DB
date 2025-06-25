#include "Server.h"

#pragma comment(lib, "ws2_32")

int main()
{
	//dll 파일 메모리에 적재
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//Listen Socket 만들기
	sockaddr_in ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(8080);

	SOCKET ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int BindResult = bind(ListenSock, (sockaddr*)&ListenSockAddr, sizeof(ListenSockAddr));
	if (BindResult <= 0)
	{
		std::cout << "Bind Error" << std::endl;
	}

	int ListenResult = listen(ListenSock, 5);
	if (ListenResult <= 0)
	{
		std::cout << "Listen Error" << std::endl;
	}

	while (true)
	{
		//Client Socket 만들기
		sockaddr_in ClientSockAddr;
		memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
		int ClientSockAddrLength = sizeof(ClientSockAddr);
		SOCKET ClientSock = accept(ListenSock, (sockaddr*)&ClientSockAddr, &ClientSockAddrLength);
		if (ClientSock == INVALID_SOCKET)
		{
			std::cout << "Accept Error" << std::endl;
		}
	}

	//dll 파일 메모리에서 삭제
	WSACleanup();

	return 0;
}