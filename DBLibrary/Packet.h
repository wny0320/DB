#pragma once

#include <iostream>
#include <WinSock2.h>
#include <vector>

static constexpr size_t TOTAL_PACKET_SIZE = 2;
static constexpr size_t CODE_SIZE = 2;
static constexpr size_t MAX_PACKET_SIZE = 4096;

enum class EEventCode
{
	CodeError = 0,
	GetSessionData = 1,
	GetPlayerData = 2,
	Max = 65535,
};

struct PlayerData
{
	bool IsSerializedFlag = false;
	unsigned short PlayerId;

	void Serialize()
	{
		IsSerializedFlag = true;
		PlayerId = htons(PlayerId);
		
	}
	void Deserialize()
	{
		IsSerializedFlag = false;
		PlayerId = ntohs(PlayerId);
	}
	bool IsSerialized()
	{
		return IsSerializedFlag;
	}
};
class Packet
{
public:
	Packet(std::vector<char> InputSerialzedPacket);
	std::vector<char> GetSerialzedPacket();
	size_t GetSerialzedPacketSize();
	bool SerializePacket(const T& PacketData);
	EEventCode DeserializePacketCode();
	std::unique_ptr<char[]> DeserializePacketData();
private:
	T SerialzedPacket;
	size_t SerialzedPacketSize;
};