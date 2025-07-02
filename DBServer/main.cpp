#include <iostream>
#include <WinSock2.h>
#include <vector>
#include "Packet.h"
#include "Server.h"

#pragma comment(lib, "ws2_32")

#define SERVER_PORT 30303

int main()
{
	//ws2_32 dll Library Load To Memory
	WSAData wsaData;
	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != 0)
	{
		std::cerr << "WSAStartup Error" << std::endl;
		return 1;
	}

	//Listen Socket Creation
	sockaddr_in ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(SERVER_PORT);

	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		std::cerr << "Listen Socket Creation Error" << std::endl;
		WSACleanup();
		return 1;
	}
	//Listen Socket Bind
	Result = bind(ListenSocket, (sockaddr*)&ListenSockAddr, sizeof(ListenSockAddr));
	if (Result <= 0)
	{
		std::cerr << "Listen Socket Bind Error" << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//Listen Socket Listen
	Result = listen(ListenSocket, 5);
	if (Result <= 0)
	{
		std::cerr << "Socket Listen Error" << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//Socket Array Structure
	fd_set MasterSet;
	//Create Server Instacne
	Server* MainServer = new Server();
	//Init fd_set
	FD_ZERO(&MasterSet);
	//ListenSocket Add To MasterSet
	FD_SET(ListenSocket, &MasterSet);


	while (true)
	{
		//Select Func Will Change fd_set, MasterSet Must Be Used After Copyed.
		fd_set ReadSet = MasterSet;

		//Remove Socket That Not Changed From Socket Array
		int SelectResult = select(0, &ReadSet, NULL, NULL, NULL);
		if (SelectResult == SOCKET_ERROR)
		{
			std::cerr << "Select TargetReadSocket Failed" << std::endl;
			break;
		}

		//Find Changes Of ReadSet Array
		if (FD_ISSET(ListenSocket, &ReadSet))
		{
			//Client Socket Creation
			sockaddr_in ClientSockAddr;
			memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
			int ClientSockAddrLength = sizeof(ClientSockAddr);
			SOCKET ClientSocket = accept(ListenSocket, (sockaddr*)&ClientSockAddr, &ClientSockAddrLength);
			if (ClientSocket == INVALID_SOCKET)
			{
				std::cerr << "Client Accept Error" << std::endl;
			}
			else
			{
				//NewClient Creation And Added To Server
				MainServer->AddSocket(ClientSocket);

				//ClientSocket Add To MasterSet
				FD_SET(ClientSocket, &MasterSet);
			}
		}

		//Each Socket In MasterSet's SocketArray
		for (u_int i = 0; i < MasterSet.fd_count; ++i)
		{
			//Get Current Socket
			SOCKET CurrentSocket = MasterSet.fd_array[i];

			if (CurrentSocket == ListenSocket)
			{
				continue;
			}

			//If CurrenSocket Exist In ReadSet's SocketArray
			if (FD_ISSET(CurrentSocket, &ReadSet))
			{
				//Find CurrenSocket At ClientMap
				auto it = MainServer->.find(CurrentSocket);
				if (it == ClientMap.end())
				{
					FD_CLR(CurrentSocket, &MasterSet);
					continue;
				}
				ClientData* MyClient = it->second;

				char Buffer[MAX_PACKET_SIZE];
				int BytesRecv = recv(MyClient->MySocket, Buffer, sizeof(Buffer), 0);
				if (BytesRecv <= 0)
				{
					std::cout << "Client Disconnected. Socket : " << CurrentSocket << std::endl;
					closesocket(CurrentSocket);
					//Remove CurrentSocket From MasterSet's SocketArray
					FD_CLR(CurrentSocket, &MasterSet);
					//Remove Dynamic Obejct
					delete it->second;
					//Remove CurrentSocket From ClientMap
					ClientMap.erase(it);

				}
				else
				{
					MyClient->RecvBuffer.insert(MyClient->RecvBuffer.end(), Buffer, Buffer + BytesRecv);
					std::cout << "Received Bytes : " << BytesRecv << " From Socket : " << MyClient->MySocket;
				}
			}
		}
	}

	closesocket(ListenSocket);

	//Remove Dynamic Objects When Code End
	for (auto const& MyPair : ClientMap)
	{
		delete MyPair.second;
	}
	ClientMap.clear();

	//ws2_32 dll Library Unload To Memory
	WSACleanup();

	return 0;
}