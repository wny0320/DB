#include "Packet.h"

Packet::Packet(std::unique_ptr<char[]> InputBuffer, size_t InputBufferSize) : Buffer(std::move(InputBuffer)), BufferSize(InputBufferSize)
{
}

char* Packet::GetBuffer()
{
	return Buffer.get();
}

size_t Packet::GetBufferSize()
{
	return BufferSize;
}

