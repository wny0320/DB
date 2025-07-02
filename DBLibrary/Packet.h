#pragma once

#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <string>

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
public:
	unsigned short PacketBodyLength;
	unsigned short PacketCode;
	bool IsSerializedFlag = false;

	PacketHeader(unsigned short Length, EEventCode Code)
	{
		PacketBodyLength = Length;
		PacketCode = (unsigned short)Code;
	}
	void Serialize()
	{
		if (IsSerializedFlag == true)
		{
			return;
		}
		IsSerializedFlag = true;
		PacketBodyLength = htons(PacketBodyLength);
		PacketCode = htons(PacketCode);
	}
	void Deserialize()
	{
		if (IsSerializedFlag == false)
		{
			return;
		}
		IsSerializedFlag = false;
		PacketBodyLength = ntohs(PacketBodyLength);
		PacketCode = ntohs(PacketCode);
	}
	bool IsSerialized() const
	{
		return IsSerializedFlag;
	}
};

struct PacketBodyBase
{
public:
	bool IsSerializedFlag = false;
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

	virtual void Serialize() override
	{
		if (IsSerializedFlag == true)
		{
			return;
		}
		IsSerializedFlag = true;
		PlayerId = htons(PlayerId);
	}
	virtual void Deserialize() override
	{
		if (IsSerializedFlag == false)
		{
			return;
		}
		IsSerializedFlag = false;
		PlayerId = ntohs(PlayerId);
	}
	unsigned short GetPlayerId() const
	{
		return PlayerId;
	}
	virtual bool IsSerialized() const override
	{
		return IsSerializedFlag;
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
	Packet(PacketHeader* InputHeader, char* InputSerialzedPacket, PacketBodyBase* InputPacket = nullptr);
	~Packet();
	PacketBodyBase* Body;
	PacketHeader* Header;

	size_t GetTotalPacketSize() const;
	bool IsPacketValid() const;
};