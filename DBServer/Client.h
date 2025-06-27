#pragma once

#include "Packet.h"

class Client
{
public:
	SOCKET MySocket;
	std::vector<char> RecvBuffer;

	Client(SOCKET Sock) : MySocket(Sock){}
};

