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

	void SendPacket(SOCKET InputSocket, Packet* InputPacket);
	void RecvPacket(SOCKET InputSocket);
	void AddSocket(SOCKET InputSocket);
	void RemoveSocket(SOCKET InputSocket);
};

