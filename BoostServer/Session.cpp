#include "Session.h"

void Session::Start() {
    std::cout << "Client Connected: " << id_ << std::endl;

    // 클라이언트로부터 데이터를 비동기적으로 수신 대기
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&Session::HandleRead, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void Session::HandleRead(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        // FlatBuffers 역직렬화
        auto verifier = flatbuffers::Verifier(reinterpret_cast<const uint8_t*>(data_), bytes_transferred);
        if (PacketData::VerifyPacketDataBuffer(verifier)) {
            auto packet = PacketData::GetPacketData(data_);
            ProcessPacket(packet);
        }
        else {
            std::cerr << "Invalid FlatBuffers data received from client " << id_ << std::endl;
        }

        // 다시 데이터 수신 대기
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&Session::HandleRead, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else {
        std::cerr << "Client Disconnected: " << id_ << ", Error: " << error.message() << std::endl;
        // 소켓은 Session 객체가 파괴될 때 자동으로 닫힙니다.
    }
}

void Session::HandleWrite(const boost::system::error_code& error) {
    if (!error) {
        // 쓰기 완료. 추가 작업이 필요하다면 여기에 구현
    }
    else {
        std::cerr << "Write Error: " << error.message() << std::endl;
    }
}

void Session::ProcessPacket(const PacketData::PacketData* packet) {
    // 패킷 타입에 따라 분기 처리
    switch (packet->type_type()) {
    case PacketData::PacketType_C2S_SessionData: {
        auto session_data = static_cast<const PacketData::C2S_SessionData*>(packet->type_as_C2S_SessionData());
        std::cout << "Received C2S_SessionData from client " << id_ << " with session_id: " << session_data->session_id() << std::endl;

        // --- RDS 연동 로직 (예시) ---
        // 여기서 session_data->session_id()를 사용해 RDS에서 유저 정보를 조회할 수 있습니다.
        // AWS SDK for C++ 를 사용하여 데이터베이스 쿼리를 실행합니다.

        // 응답 패킷 생성 (FlatBuffers)
        flatbuffers::FlatBufferBuilder builder;
        std::vector<uint32_t> player_ids = { 1, 2, 3 };
        auto all_player_id_offset = builder.CreateVector(player_ids);

        auto s2c_session_data = PacketData::CreateS2C_SessionData(builder, session_data->session_id(), 1, all_player_id_offset, 1000, 1);
        auto packet_data = PacketData::CreatePacketData(builder, 0, PacketData::PacketType_S2C_SessionData, s2c_session_data.Union());
        builder.Finish(packet_data);

        // 클라이언트에 응답 전송
        boost::asio::async_write(socket_, boost::asio::buffer(builder.GetBufferPointer(), builder.GetSize()),
            boost::bind(&Session::HandleWrite, shared_from_this(),
                boost::asio::placeholders::error));
        break;
    }
                                               // 다른 패킷 타입들에 대한 처리 추가
    default:
        std::cout << "Received unknown packet type from client " << id_ << std::endl;
        break;
    }
}