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
	int PlayerId; // Changed to int
	char Username[101]; // Added as fixed-size char array
	unsigned int PlayerHp; // Added
	unsigned int PlayerStamina; // Added

	// Default constructor
	PlayerData() : PlayerId(0), PlayerHp(0), PlayerStamina(0) {
        memset(Username, 0, sizeof(Username)); // Initialize char array
    }

	virtual void Serialize() override
	{
		if (bIsSerialized == true)
		{
			return;
		}
		bIsSerialized = true;
		PlayerId = htonl(PlayerId); // Changed to htonl
		PlayerHp = htonl(PlayerHp); // Added
		PlayerStamina = htonl(PlayerStamina); // Added
	}
	virtual void Deserialize() override
	{
		if (bIsSerialized == false)
		{
			return;
		}
		bIsSerialized = false;
		PlayerId = ntohl(PlayerId); // Changed to ntohl
		PlayerHp = ntohl(PlayerHp); // Added
		PlayerStamina = ntohl(PlayerStamina); // Added
	}
	int GetPlayerId() const // Return type changed to int
	{
		return PlayerId;
	}
	virtual bool IsSerialized() const override
	{
		return bIsSerialized;
	}
	virtual size_t GetPacketBodySize() const override
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