#include "Server.h"

Server::Server()
{
}

Server::~Server()
{
}

void Server::SendPacket(int ClientSocket)
{
	char Buffer[1024] = { 0, };
	send(ClientSocket, Buffer, sizeof(Buffer), 0);
}

void Server::RecvPacket(int ClientSocket, unsigned short PacketLength)
{
	if (PacketLength == 0 || PacketLength > MAX_PACKET_SIZE)
	{
		std::cout << "Invalid Packet Length Received" << std::endl;
		RemoveSocket(ClientSocket);
		return;
	}
	std::unique_ptr<char[]> Buffer;
	int RecvTotalByte = 0;
	do
	{
		int RecvByte = recv(ClientSocket, Buffer.get() + RecvTotalByte, PacketLength - RecvTotalByte, 0);
		RecvTotalByte += RecvByte;
	} while (RecvTotalByte < PacketLength);

	Packet RecvPacket(std::move(Buffer), PacketLength);


	return;
}

unsigned short Server::RecvPacketLength(int ClientSocket)
{
	char PacketLengthChar[TOTAL_PACKET_SIZE] = { 0, };
	unsigned short PacketLength = 0;

	int RecvTotalByte = 0;
	do
	{
		int RecvByte = recv(ClientSocket, PacketLengthChar + RecvTotalByte, TOTAL_PACKET_SIZE - RecvTotalByte, 0);
		RecvTotalByte += RecvByte;
	} while (RecvTotalByte < TOTAL_PACKET_SIZE);

	PacketLength = ntohs(*reinterpret_cast<unsigned short*>(PacketLengthChar));

	return PacketLength;
}

void Server::AddSocket(int ClientSocket)
{
}

void Server::RemoveSocket(int ClientSocket)
{
	//SockVector.
	//SockVector.erase();
	closesocket(ClientSocket);
}
