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
	std::vector<SOCKET> MyClientVector;
	std::unordered_map<SOCKET, Packet*> MyTaskMap;

	void SendPacket(Packet* InputPacket);
	void RecvPacket(SOCKET InputSocket, Packet* InputPacket);
	void AddClient(SOCKET InputSocket);
	void RemoveClient(SOCKET InputSocket);
};

