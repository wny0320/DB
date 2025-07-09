#pragma once

#include <thread>
#include <boost/asio.hpp>
#include <unordered_map>
#include <Common.h>

using boost::asio::ip::tcp;

class Client;

class BoostServer
{
public:
	BoostServer();
	~BoostServer();
	std::vector<Client> ClientVector;

	void SendPacket(tcp::socket* InSocket);
	bool RecvPacket(tcp::socket* InSocket);
	void AddClient(tcp::socket* InSocket);
	void RemoveClient(tcp::socket* InSocket);
};

