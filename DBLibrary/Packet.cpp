#include "Packet.h"

Packet::Packet(PacketHeader* InputHeader, char* InputSerialzedPacket, PacketBodyBase* InputPacket)
{
	Header = InputHeader;
	EEventCode Code = (EEventCode)Header->PacketCode;
	if (InputSerialzedPacket == nullptr)
	{
		Body = InputPacket;
	}
	else if(InputPacket == nullptr)
	{
		switch (Code)
		{
		case EEventCode::CodeError:
			break;
		case EEventCode::GetSessionData:

			break;
		case EEventCode::GetPlayerData:
			PlayerData* Data = new PlayerData();
			memcpy(Data, InputSerialzedPacket, sizeof(PlayerData));
			Data->Deserialize();
			Body = Data;
			delete[] InputSerialzedPacket;
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
