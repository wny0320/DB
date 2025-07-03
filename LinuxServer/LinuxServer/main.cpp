#include <iostream>
#include <vector>
#include <cstring> // For memset
#include <sys/socket.h> // For socket, bind, listen, accept, send, recv
#include <netinet/in.h> // For sockaddr_in, INADDR_ANY, htons
#include <unistd.h>     // For close
#include <arpa/inet.h>  // For htons, ntohs
#include <sys/select.h> // For fd_set, select

#include "Packet.h"
#include "Server.h"

#define SERVER_PORT 30303

int main()
{
	// No WSAStartup/WSACleanup needed for POSIX sockets

	//Listen Socket Creation
	sockaddr_in ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(SERVER_PORT);

	int ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == -1) // INVALID_SOCKET is -1 in POSIX
	{
		std::cerr << "Listen Socket Creation Error" << std::endl;
		return 1;
	}

	// Optional: Set SO_REUSEADDR to allow immediate reuse of the port
    int optval = 1;
    if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        std::cerr << "setsockopt SO_REUSEADDR failed" << std::endl;
        close(ListenSocket);
        return 1;
    }

	//Listen Socket Bind
	int Result = bind(ListenSocket, (sockaddr*)&ListenSockAddr, sizeof(ListenSockAddr));
	if (Result == -1)
	{
		std::cerr << "Listen Socket Bind Error" << std::endl;
		close(ListenSocket);
		return 1;
	}

	//Listen Socket Listen
	Result = listen(ListenSocket, 5);
	if (Result == -1)
	{
		std::cerr << "Socket Listen Error" << std::endl;
		close(ListenSocket);
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

	int MaxSocket = ListenSocket; // Track the highest-numbered file descriptor for select()

	while (true)
	{
		//Select Func Will Change fd_set, MasterSet Must Be Used After Copyed.
		fd_set ReadSet = MasterSet;

		//Remove Socket That Not Changed From Socket Array
		int SelectResult = select(MaxSocket + 1, &ReadSet, NULL, NULL, NULL); // nfds is max_fd + 1
		if (SelectResult == -1) // SOCKET_ERROR is -1 in POSIX
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
			socklen_t ClientSockAddrLength = sizeof(ClientSockAddr); // socklen_t for POSIX accept
			int ClientSocket = accept(ListenSocket, (sockaddr*)&ClientSockAddr, &ClientSockAddrLength);
			if (ClientSocket == -1) // INVALID_SOCKET is -1 in POSIX
			{
				std::cerr << "Client Accept Error" << std::endl;
			}
			else
			{
				//NewClient Creation And Added To Server
				MainServer->AddSocket(ClientSocket);

				//ClientSocket Add To MasterSet
				FD_SET(ClientSocket, &MasterSet);
				// Update MaxSocket if new client socket is higher
				if (ClientSocket > MaxSocket)
				{
					MaxSocket = ClientSocket;
				}
				std::cout << "New Client Connected. Socket : " << ClientSocket << std::endl;
			}
		}

		//Each Socket In MasterSet's SocketArray
		// Create a copy of the socket vector to iterate over safely
        // because RecvPacket might trigger RemoveSocket, which modifies the vector.
		std::vector<int> SocketsToCheck = MainServer->SocketVector;
		for (int CurrentSocket : SocketsToCheck)
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
					// Re-calculate MaxSocket if the disconnected socket was the max
					if (CurrentSocket == MaxSocket)
					{
						MaxSocket = ListenSocket;
						for (int s : MainServer->SocketVector)
						{
							if (s > MaxSocket)
							{
								MaxSocket = s;
							}
						}
					}
				}
			}
		}
	}

	close(ListenSocket);

	//Remove Dynamic Objects When Code End
	delete MainServer;

	// No WSACleanup needed for POSIX sockets

	return 0;
}