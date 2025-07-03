#pragma once

#include <thread>
#include <unordered_map>
#include <vector>
#include <sys/socket.h> // For socket, bind, listen, accept, send, recv
#include <netinet/in.h> // For sockaddr_in, INADDR_ANY, htons
#include <unistd.h>     // For close

class Packet;

class Server
{
public:
	Server();
	~Server();
	std::vector<int> SocketVector;
	std::unordered_map<int, Packet*> RecvPacketMap;

	void SendPacket(int InSocket, Packet* InPacket);
	bool RecvPacket(int InSocket);
	void AddSocket(int InSocket);
	void RemoveSocket(int InSocket);

private:
	sql::Driver* Driver;
	sql::Connection* Connection;

	bool InitializeDatabase();
	bool GetPlayerDataFromDB(unsigned short InPlayerId, PlayerData& OutPlayerData);
};