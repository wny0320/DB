#include "Packet.h"


Packet::Packet(std::unique_ptr<char[]> InputBuffer, size_t InputBufferSize) : SerialzedPacket(std::move(InputBuffer)), SerialzedPacketSize(InputBufferSize)
{
}

char* Packet::GetSerialzedPacket()
{
	return SerialzedPacket.get();
}

size_t Packet::GetSerialzedPacketSize()
{
	return SerialzedPacketSize;
}

EEventCode Packet::DeserializePacketCode()
{
	if (SerialzedPacket = NULL)
	{
		return EEventCode::CodeError;
	}
	char CodeChar[CODE_SIZE] = { 0, };
	unsigned short Code = 0;

	memcpy(CodeChar, &SerialzedPacket, CODE_SIZE);
	
	Code = ntohs(*reinterpret_cast<unsigned short*>(CodeChar));

	return (EEventCode)(Code);
}

std::unique_ptr<char[]> Packet::DeserializePacketData()
{
	if (SerialzedPacket = NULL)
	{
		return NULL;
	}
	std::unique_ptr<char[]> Data = std::make_unique<char[]>(Packet::SerialzedPacketSize);

	memcpy(Data.get(), &(Packet::SerialzedPacket), SerialzedPacketSize - CODE_SIZE);

	return std::move(Data);
}

