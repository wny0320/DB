#include <algorithm>

#include "Server.h"
#include "Packet.h"

Server::Server()
{
}

Server::~Server()
{
}

void Server::SendPacket(SOCKET InputSocket, Packet* InputPacket)
{
	if (InputPacket->Header->IsSerialized() == false)
	{
		InputPacket->Header->Serialize();
	}
	if (InputPacket->Body->IsSerialized() == false)
	{
		InputPacket->Body->Serialize();
	}
	//Send Header
	send(InputSocket, (char*)InputPacket->Header, sizeof(PacketHeader), 0);
	//Send Body
	send(InputSocket, (char*)InputPacket->Body, sizeof(InputPacket->Body->GetPacketBodySize()), 0);
}

void Server::RecvPacket(SOCKET InputSocket)
{
	PacketHeader* NewHeader = new PacketHeader();
	int RecvBytes = recv(InputSocket, (char*)&NewHeader, sizeof(PacketHeader), 0);
	if (RecvBytes <= 0)
	{
		RemoveSocket(InputSocket);
		return;
	}
	NewHeader->Deserialize();
	PacketBodyBase* NewBody = nullptr;
	switch ((EEventCode)(NewHeader->PacketCode))
	{
	case EEventCode::CodeError:
		break;
	case EEventCode::GetSessionData:
		break;
	case EEventCode::GetPlayerData:
		NewBody = new PlayerData();
		break;
	case EEventCode::Max:
		break;
	default:
		break;
	}
	
	RecvBytes = recv(InputSocket, (char*)&NewBody, NewBody->GetPacketBodySize(), 0);
	NewBody->Deserialize();
	Packet* NewPacket = new Packet(NewHeader, NewBody);
}

void Server::AddSocket(SOCKET InputSocket)
{
	SocketVector.push_back(InputSocket);
}

void Server::RemoveSocket(SOCKET InputSocket)
{
	auto it = std::find(SocketVector.begin(), SocketVector.end(), InputSocket);
	SocketVector.erase(it);
	closesocket(InputSocket);
}
