#include "Session.h"

void Session::Start() {
    std::cout << "Client Connected: " << id_ << std::endl;

    // Ŭ���̾�Ʈ�κ��� �����͸� �񵿱������� ���� ���
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&Session::HandleRead, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void Session::HandleRead(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        // FlatBuffers ������ȭ
        auto verifier = flatbuffers::Verifier(reinterpret_cast<const uint8_t*>(data_), bytes_transferred);
        if (PacketData::VerifyPacketDataBuffer(verifier)) {
            auto packet = PacketData::GetPacketData(data_);
            ProcessPacket(packet);
        }
        else {
            std::cerr << "Invalid FlatBuffers data received from client " << id_ << std::endl;
        }

        // �ٽ� ������ ���� ���
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&Session::HandleRead, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else {
        std::cerr << "Client Disconnected: " << id_ << ", Error: " << error.message() << std::endl;
        // ������ Session ��ü�� �ı��� �� �ڵ����� �����ϴ�.
    }
}

void Session::HandleWrite(const boost::system::error_code& error) {
    if (!error) {
        // ���� �Ϸ�. �߰� �۾��� �ʿ��ϴٸ� ���⿡ ����
    }
    else {
        std::cerr << "Write Error: " << error.message() << std::endl;
    }
}

void Session::ProcessPacket(const PacketData::PacketData* packet) {
    // ��Ŷ Ÿ�Կ� ���� �б� ó��
    switch (packet->type_type()) {
    case PacketData::PacketType_C2S_SessionData: {
        auto session_data = static_cast<const PacketData::C2S_SessionData*>(packet->type_as_C2S_SessionData());
        std::cout << "Received C2S_SessionData from client " << id_ << " with session_id: " << session_data->session_id() << std::endl;

        // --- RDS ���� ���� (����) ---
        // ���⼭ session_data->session_id()�� ����� RDS���� ���� ������ ��ȸ�� �� �ֽ��ϴ�.
        // AWS SDK for C++ �� ����Ͽ� �����ͺ��̽� ������ �����մϴ�.

        // ���� ��Ŷ ���� (FlatBuffers)
        flatbuffers::FlatBufferBuilder builder;
        std::vector<uint32_t> player_ids = { 1, 2, 3 };
        auto all_player_id_offset = builder.CreateVector(player_ids);

        auto s2c_session_data = PacketData::CreateS2C_SessionData(builder, session_data->session_id(), 1, all_player_id_offset, 1000, 1);
        auto packet_data = PacketData::CreatePacketData(builder, 0, PacketData::PacketType_S2C_SessionData, s2c_session_data.Union());
        builder.Finish(packet_data);

        // Ŭ���̾�Ʈ�� ���� ����
        boost::asio::async_write(socket_, boost::asio::buffer(builder.GetBufferPointer(), builder.GetSize()),
            boost::bind(&Session::HandleWrite, shared_from_this(),
                boost::asio::placeholders::error));
        break;
    }
                                               // �ٸ� ��Ŷ Ÿ�Ե鿡 ���� ó�� �߰�
    default:
        std::cout << "Received unknown packet type from client " << id_ << std::endl;
        break;
    }
}