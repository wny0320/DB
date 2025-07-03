#include "Packet.h"
#include <cstring> // For memcpy

Packet::Packet(PacketHeader* InHeader, PacketBodyBase* InPacket, char* InSerialzedPacket)
{
	Header = InHeader;
	EEventCode Code = (EEventCode)Header->PacketCode;
	if (InSerialzedPacket == nullptr)
	{
		Body = InPacket;
	}
	else if(InPacket == nullptr)
	{
		switch (Code)
		{
		case EEventCode::CodeError:
			break;
		case EEventCode::GetSessionData:

			break;
		case EEventCode::GetPlayerData:
			{
				PlayerData* Data = new PlayerData(InSerialzedPacket);
				Data->Deserialize();
				Body = Data;
				delete[] InSerialzedPacket;
			}
			break;
		case EEventCode::Max:
			break;
		default:
			break;
		}
	}
}

Packet::~Packet()
{
	delete Body;
	delete Header;
	Header = nullptr;
}

size_t Packet::GetTotalPacketSize() const
{
	return sizeof(PacketHeader) + (Body ? Body->GetPacketBodySize() : 0);
}

bool Packet::IsPacketValid() const
{
	return (Body != nullptr) && (Header != nullptr) && (Header->PacketBodyLength == Body->GetPacketBodySize());
}
