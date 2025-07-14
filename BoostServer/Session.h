#pragma once

#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "PacketData_generated.h"

using boost::asio::ip::tcp;

class Session : public boost::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context& IoContext, uint32_t InId) : Socket(IoContext), Id(InId) {}
    void HandleNicknameCheckRequest(const std::string& nickname);
    tcp::socket& GetSocket();
    void Start();

private:
    void HandleRead(const boost::system::error_code& ErrorCode, size_t BytesTransferred);
    void HandleWrite(const boost::system::error_code& ErrorCode);

    void ProcessPacket(const PacketData::Packet* InPacket);

private:
    tcp::socket Socket;
    uint32_t Id;
    enum { MaxLength = 1024 };
    char DataBuffer[MaxLength];
};
