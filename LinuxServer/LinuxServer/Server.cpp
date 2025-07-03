#include <algorithm>
#include <unistd.h> // For close
#include <errno.h>  // For errno

#include "Server.h"
#include "Packet.h"

Server::Server()
{
}

Server::~Server()
{
	for (auto const& pair : RecvPacketMap)
	{
		delete pair.second;
	}
	RecvPacketMap.clear();

	for (int s : SocketVector)
	{
		close(s);
	}
	SocketVector.clear();
}

void Server::SendPacket(int InSocket, Packet* InPacket)
{
	if (InPacket->Header->IsSerialized() == false)
	{
		InPacket->Header->Serialize();
	}
	if (InPacket->Body->IsSerialized() == false)
	{
		InPacket->Body->Serialize();
	}
	//Send Header
	if (send(InSocket, (char*)InPacket->Header, sizeof(PacketHeader), 0) < 0)
	{
		RemoveSocket(InSocket);
		return;
	}
	//Send Body
	if (send(InSocket, (char*)InPacket->Body, InPacket->Body->GetPacketBodySize(), 0) < 0)
	{
		RemoveSocket(InSocket);
		return;
	}
}

bool Server::RecvPacket(int InSocket)
{
	PacketHeader* NewHeader = new PacketHeader();
	int RecvBytes = recv(InSocket, (char*)NewHeader, sizeof(PacketHeader), 0);
	if (RecvBytes <= 0)
	{
		RemoveSocket(InSocket);
		delete NewHeader;
		return false;
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
	
	//If Body Null, Recv Packet And Remove
	if (NewBody == nullptr)
	{
		if (NewHeader->PacketBodyLength > 0)
		{
			char* TempBuffer = new char[NewHeader->PacketBodyLength];
			recv(InSocket, TempBuffer, NewHeader->PacketBodyLength, 0);
			delete[] TempBuffer;
		}
		delete NewHeader;
		return true; // Still a valid connection, just an unhandled packet
	}

	RecvBytes = recv(InSocket, (char*)NewBody, NewHeader->PacketBodyLength, 0);
	//Recv Failed
	if (RecvBytes <= 0)
	{
		RemoveSocket(InSocket);
		delete NewHeader;
		delete NewBody;
		return false;
	}

	NewBody->Deserialize();
	Packet* NewPacket = new Packet(NewHeader, NewBody);
	//Emplace Avoid Copy
	RecvPacketMap.emplace(InSocket, NewPacket);
	return true;
}

void Server::AddSocket(int InSocket)
{
	SocketVector.push_back(InSocket);
}

void Server::RemoveSocket(int InSocket)
{
	auto It = std::find(SocketVector.begin(), SocketVector.end(), InSocket);
	if (It != SocketVector.end())
	{
		SocketVector.erase(It);
	}

	auto MapIt = RecvPacketMap.find(InSocket);
	if (MapIt != RecvPacketMap.end())
	{
		delete MapIt->second;
		RecvPacketMap.erase(MapIt);
	}

	close(InSocket);
}