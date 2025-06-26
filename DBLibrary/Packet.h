#pragma once

static constexpr size_t TOTAL_PACKET_SIZE = 2;
static constexpr size_t CODE_SIZE = 2;
static constexpr size_t MAX_PACKET_SIZE = 4096;

#include <iostream>
#include <WinSock2.h>

enum class EEventCode
{
	CodeError = 0,
	GetSessionData = 1,
	GetPlayerData = 2,
	Max = 65535,
};

struct PlayerData
{

};
class Packet
{
public:
	Packet(std::unique_ptr<char[]> InputSerialzedPacket = NULL, size_t InputSerialzedPacketSize = NULL);
	char* GetSerialzedPacket();
	size_t GetSerialzedPacketSize();
	template<typename T>
	bool SerializePacket(const T& PacketData);
	EEventCode DeserializePacketCode();
	std::unique_ptr<char[]> DeserializePacketData();
private:
	std::unique_ptr<char[]> SerialzedPacket;
	size_t SerialzedPacketSize;
};

template<typename T>
inline bool Packet::SerializePacket(const T& PacketData)
{
	const size_t PacketSize = sizeof(T);
	std::unique_ptr<char[]> Buffer = std::make_unique<char[]>(PacketSize);

	std::memcpy(Buffer.get(), &PacketData, PacketSize);

	SerialzedPacket = std::move(Buffer);
	if (SerialzedPacket == NULL)
	{
		return false;
	}
	else
	{
		return true;
	}
};