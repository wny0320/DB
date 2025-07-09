#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include "BoostServer.h"
#include "Common.h"

using boost::asio::ip::tcp;

#define SERVER_PORT 30303

int main()
{
	//ws2_32 dll Library Load To Memory
	WSAData WsaData;
	int Result = WSAStartup(MAKEWORD(2, 2), &WsaData);
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

	tcp::socket ListenSocket = tcp::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
	BoostServer* MainServer = new BoostServer();
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
				MainServer->AddClient(ClientSocket);

				//ClientSocket Add To MasterSet
				FD_SET(ClientSocket, &MasterSet);
				std::cout << "New Client Connected. Socket : " << ClientSocket << std::endl;
			}
		}

		//Each Socket In MasterSet's SocketArray
		// Create a copy of the socket vector to iterate over safely
		// because RecvPacket might trigger RemoveSocket, which modifies the vector.
		std::vector<SOCKET> SocketsToCheck = MainServer->ClientVector;
		for (SOCKET CurrentSocket : SocketsToCheck)
		{
			//If CurrenSocket Exist In ReadSet's SocketArray
			if (FD_ISSET(CurrentSocket, &ReadSet))
			{
				//Find CurrenSocket At ClientMap
				if (!MainServer->RecvPacket(CurrentSocket))
				{
					// RecvPacket returned false, meaning connection is closed.
					// RemoveSocket was already called inside RecvPacket.
					// We just need to remove it from the master set for select().
					FD_CLR(CurrentSocket, &MasterSet);
					std::cout << "Client Disconnected. Socket : " << CurrentSocket << std::endl;
				}
			}
		}
	}

	closesocket(ListenSocket);

	//Remove Dynamic Objects When Code End
	delete MainServer;

	//ws2_32 dll Library Unload To Memory
	WSACleanup();

	return 0;
}