#include "Session.h"

Session::Session(boost::asio::io_context& IoContext, uint32_t InId)
    : Socket(IoContext), Id(InId)
{
}

tcp::socket& Session::GetSocket()
{
    return Socket;
}

void Session::Start()
{
    std::cout << "Client Connected. Session ID: " << Id << std::endl;

    Socket.async_read_some(boost::asio::buffer(DataBuffer, MaxLength),
        boost::bind(&Session::HandleRead, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void Session::HandleRead(const boost::system::error_code& ErrorCode, size_t BytesTransferred)
{
    if (!ErrorCode)
    {
        auto Verifier = flatbuffers::Verifier(reinterpret_cast<const uint8_t*>(DataBuffer), BytesTransferred);

        // [변경] 버퍼 검증 함수가 VerifyPacketBuffer로 변경되었습니다.
        if (PacketData::VerifyPacketBuffer(Verifier))
        {
            // [변경] 패킷을 가져오는 함수가 GetPacket으로 변경되었습니다.
            auto ReceivedPacket = PacketData::GetPacket(DataBuffer);
            ProcessPacket(ReceivedPacket);
        }
        else
        {
            std::cerr << "Invalid FlatBuffers data received from Session ID: " << Id << std::endl;
        }

        Socket.async_read_some(boost::asio::buffer(DataBuffer, MaxLength),
            boost::bind(&Session::HandleRead, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        std::cerr << "Session Disconnected. ID: " << Id << ", Error: " << ErrorCode.message() << std::endl;
    }
}

void Session::HandleWrite(const boost::system::error_code& ErrorCode)
{
    if (!ErrorCode)
    {
        // 쓰기 완료
    }
    else
    {
        std::cerr << "Write Error on Session ID " << Id << ": " << ErrorCode.message() << std::endl;
    }
}

// [변경] 함수의 인자 타입이 Packet*로 변경되었습니다.
void Session::ProcessPacket(const PacketData::Packet* InPacket)
{
    // InPacket의 타입은 이제 PacketData::Packet* 입니다.
    switch (InPacket->type_type())
    {
    case PacketData::PacketType_C2S_SessionData:
    {
        auto SessionData = static_cast<const PacketData::C2S_SessionData*>(InPacket->type_as_C2S_SessionData());
        std::cout << "Received C2S_SessionData from Session ID " << Id << " with session_id: " << SessionData->session_id() << std::endl;

        flatbuffers::FlatBufferBuilder Builder;
        std::vector<uint32_t> PlayerIds = { 1, 2, 3 };
        auto AllPlayerIdOffset = Builder.CreateVector(PlayerIds);

        auto S2CSessionData = PacketData::CreateS2C_SessionData(Builder, SessionData->session_id(), 1, AllPlayerIdOffset, 1000, 1);

        auto ResponsePacket = PacketData::CreatePacket(Builder, 0, PacketData::PacketType_S2C_SessionData, S2CSessionData.Union());
        Builder.Finish(ResponsePacket);

        boost::asio::async_write(Socket, boost::asio::buffer(Builder.GetBufferPointer(), Builder.GetSize()),
            boost::bind(&Session::HandleWrite, shared_from_this(),
                boost::asio::placeholders::error));
        break;
    }
    default:
        std::cout << "Received unknown packet type from Session ID " << Id << std::endl;
        break;
    }
}
