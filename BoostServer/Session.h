#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "PacketData_generated.h"

using boost::asio::ip::tcp;

class Session : public boost::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context& IoContext, uint32_t Id)
        : Socket(IoContext), Id(Id) {
    }

    void SetClientIP(const std::string& ip_address) { ClientIP = ip_address; }
    tcp::socket& GetSocket() { return Socket; }
    uint32_t GetId() const { return Id; }
    void Start();

private:
    void HandleRead(const boost::system::error_code& ErrorCode, size_t BytesTransferred);
    void HandleWrite(const boost::system::error_code& ErrorCode);
    void ProcessPacket(const uint8_t* data, size_t size);

    void ProcessCreateSessionRequest(const PacketData::C2S_CreateSession* req);

    void ProcessJoinSessionRequest(const PacketData::C2S_JoinSession* req);

    void BuildAndUpdateSession(uint32_t session_id, flatbuffers::FlatBufferBuilder& builder);

    tcp::socket Socket;
    uint32_t Id;
    enum { MaxLength = 1024 };
    char DataBuffer[MaxLength];
    std::string ClientIP;

    std::vector<uint8_t> ReadBuffer;
};
