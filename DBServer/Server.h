#pragma once

#include <thread>
#include <WinSock2.h>
#include "Packet.h"

class Server
{
public:
	Server();
	~Server();

	void SendPacket(int ClientSocket);

	void RecvPacket(int ClientSocket, unsigned short PacketLength);

	unsigned short RecvPacketLength(int Client);

	void AddSocket(int ClientSocket);

	void RemoveSocket(int ClientSocket);
private:
	std::vector<SOCKET> SockVector;
	std::vector<std::pair<SOCKET, EEventCode>> TaskVector;

};

