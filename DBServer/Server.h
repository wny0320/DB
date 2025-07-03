#pragma once

#include <thread>
#include <WinSock2.h>
#include <unordered_map>

class Packet;

class Server
{
public:
	Server();
	~Server();
	std::vector<SOCKET> SocketVector;
	std::unordered_map<SOCKET, Packet*> RecvPacketMap;

	void SendPacket(SOCKET InSocket, Packet* InPacket);
	bool RecvPacket(SOCKET InSocket);
	void AddSocket(SOCKET InSocket);
	void RemoveSocket(SOCKET InSocket);
};

