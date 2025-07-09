#pragma once

#include <boost/asio.hpp>
#include <vector>

using boost::asio::ip::tcp;

struct Client
{
	uint32_t Id;
	tcp::socket Socket;
	char Buffer[1024];

	Client(uint32_t InId, tcp::socket InSocket, char* InBuffer) : Id(InId), Socket(std::move(InSocket)) {}
	~Client()
	{
		Socket.shutdown(Socket.shutdown_both);
		Socket.close();
	}
};

