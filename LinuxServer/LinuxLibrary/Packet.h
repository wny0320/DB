#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <arpa/inet.h> // For htons, ntohs on Linux
#include <cstring> // For memcpy

#define MAX_PACKET_SIZE	4096

enum class EEventCode : unsigned short
{
	CodeError = 0,	
	GetSessionData = 1,
	GetPlayerData = 2,
	AddPlayerData = 3,
	AddSessionData = 4,
	Login = 5,
	Logout = 6,
	LoginResult = 7,
	LogoutResult = 8,



	Max = 65535,
};

struct PacketHeader
{
private:
	bool bIsSerialized = false;
public:
	unsigned short PacketBodyLength;
	unsigned short PacketCode;

	PacketHeader(unsigned short InLength = 0, EEventCode InCode = EEventCode::CodeError)
	{
		PacketBodyLength = InLength;
		PacketCode = (unsigned short)InCode;
	}
	void Serialize()
	{
		if (bIsSerialized == true)
		{
			return;
		}
		bIsSerialized = true;
		PacketBodyLength = htons(PacketBodyLength);
		PacketCode = htons(PacketCode);
	}
	void Deserialize()
	{
		if (bIsSerialized == false)
		{
			return;
		}
		bIsSerialized = false;
		PacketBodyLength = ntohs(PacketBodyLength);
		PacketCode = ntohs(PacketCode);
	}
	bool IsSerialized() const
	{
		return bIsSerialized;
	}
};

struct PacketBodyBase
{
protected:
	bool bIsSerialized = false;
public:
	unsigned short PacketData;

	virtual ~PacketBodyBase() = default;
	virtual void Serialize() = 0;
	virtual void Deserialize() = 0;
	virtual bool IsSerialized() const = 0;
	virtual size_t GetPacketBodySize() const = 0;
};

#pragma pack(push, 1)
struct PlayerData : PacketBodyBase
{
public:
	unsigned short PlayerId;

	// Constructor to initialize from serialized data
	PlayerData(char* InSerializedData = nullptr)
	{
		if (InSerializedData)
		{
			// Directly copy the bytes. This assumes the serialized data
			// has the same layout as PlayerData. Be cautious with this.
			memcpy(this, InSerializedData, sizeof(PlayerData));
		}
	}

	virtual void Serialize() override
	{
		if (bIsSerialized == true)
		{
			return;
		}
		bIsSerialized = true;
		PlayerId = htons(PlayerId);
	}
	virtual void Deserialize() override
	{
		if (bIsSerialized == false)
		{
			return;
		}
		bIsSerialized = false;
		PlayerId = ntohs(PlayerId);
	}
	unsigned short GetPlayerId() const
	{
		return PlayerId;
	}
	virtual bool IsSerialized() const override
	{
		return bIsSerialized;
	}
	virtual size_t GetPacketBodySize() const
	{
		return sizeof(PlayerData);
	}
};
#pragma pack(pop)

class Packet
{
public:
	Packet(PacketHeader* InHeader, PacketBodyBase* InPacket = nullptr, char* InSerialzedPacket = nullptr);
	~Packet();
	PacketBodyBase* Body;
	PacketHeader* Header;

	size_t GetTotalPacketSize() const;
	bool IsPacketValid() const;
};