#pragma once

static constexpr size_t TOTAL_PACKET_SIZE = 2;
static constexpr size_t CODE_SIZE = 2;
static constexpr size_t MAX_PACKET_SIZE = 4096;

#include <iostream>

enum class EEventCode
{
	Max = 65535,
};

class Packet
{
public:
	Packet(std::unique_ptr<char[]> InputBuffer, size_t InputBufferSize);
	char* GetBuffer();
	size_t GetBufferSize();
private:
	std::unique_ptr<char[]> Buffer;
	size_t BufferSize;
};