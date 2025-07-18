#include "Session.h"
#include "DatabaseManager.h" 
#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <memory> 
#include <vector> 
#include <cstdlib>

void Session::Start()
{
    std::cout << "Client Connected. Session ID: " << Id << ", IP: " << ClientIP << std::endl;
    Socket.async_read_some(boost::asio::buffer(DataBuffer, MaxLength),
        boost::bind(&Session::HandleRead, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

// [����] HandleRead ���� ��ü ����
void Session::HandleRead(const boost::system::error_code& ErrorCode, size_t BytesTransferred)
{
    if (!ErrorCode && BytesTransferred > 0)
    {
        // 1. ���� ���� �����͸� ReadBuffer�� ���� �߰��մϴ�.
        ReadBuffer.insert(ReadBuffer.end(), DataBuffer, DataBuffer + BytesTransferred);

        const int32_t HeaderSize = 4; // ��� ������� 4����Ʈ

        // 2. ���ۿ� ������ ��Ŷ�� �ϳ� �̻� ������ �� �����Ƿ�, while ������ ����մϴ�.
        while (ReadBuffer.size() >= HeaderSize)
        {
            // 3. ������� ��Ŷ ��ü ���̸� �н��ϴ�.
            int32_t PacketLength = 0;
            memcpy(&PacketLength, ReadBuffer.data(), HeaderSize);

            // 4. ��Ŷ�� ������ �����ߴ��� Ȯ���մϴ�.
            if (ReadBuffer.size() >= PacketLength)
            {
                // 5. ������ ��Ŷ�� '���� ������' �κи� ProcessPacket���� �Ѱ� ó���մϴ�.
                // (����� ������ ������ ������, ���� ������ ����)
                ProcessPacket(ReadBuffer.data() + HeaderSize, PacketLength - HeaderSize);

                // 6. ���ۿ��� ó���� ��Ŷ(��� ����)�� �����մϴ�.
                ReadBuffer.erase(ReadBuffer.begin(), ReadBuffer.begin() + PacketLength);
            }
            else
            {
                // ���� ��Ŷ�� �� �������� ����. ���� read�� ��ٸ��ϴ�.
                break;
            }
        }

        // ���� �����͸� ����ؼ� �񵿱������� ��ٸ��ϴ�.
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
    if (ErrorCode)
    {
        std::cerr << "Write Error on Session ID " << Id << ": " << ErrorCode.message() << std::endl;
    }
}

// [���� �Ϸ�] ProcessPacket() �Լ�
void Session::ProcessPacket(const uint8_t* data, size_t size)
{
    auto Verifier = flatbuffers::Verifier(data, size);
    if (!PacketData::VerifyPacketBuffer(Verifier))
    {
        // �� �α״� ���� ������ �����Ͱ� �ջ�Ǿ��� ���� �߻��մϴ�.
        std::cerr << "Invalid FlatBuffers data received from Session ID: " << Id << std::endl;
        return;
    }

    auto InPacket = PacketData::GetPacket(data);
    switch (InPacket->packet_pay_load_type())
    {
    case PacketData::PacketPayload_C2S_CreateSession:
    {
        auto req = InPacket->packet_pay_load_as_C2S_CreateSession();
        if (req) {
            ProcessCreateSessionRequest(req);
        }
        break;
    }
    case PacketData::PacketPayload_C2S_JoinSession:
    {
        auto req = InPacket->packet_pay_load_as_C2S_JoinSession();
        if (req) {
            ProcessJoinSessionRequest(req);
        }
        break;
    }
    // �ٸ� ��Ŷ Ÿ�� ó��...
    default:
        std::cout << "Received unknown packet type from Session ID " << Id << std::endl;
        break;
    }
}

// [���� �Ϸ�] ProcessCreateSessionRequest() �Լ�
void Session::ProcessCreateSessionRequest(const PacketData::C2S_CreateSession* req)
{
    std::string hostNickname = req->host_nickname()->str();
    // Ŭ���̾�Ʈ�� ���� IP ��� ������ ������ IP�� ����մϴ�.
    std::string hostIpAddress = this->ClientIP;

    std::cout << "���� ���� ��û | Nickname: " << hostNickname << ", IP: " << hostIpAddress << std::endl;

    uint32_t newSessionId = 0;
    flatbuffers::FlatBufferBuilder builder;

    sql::Connection& con = DatabaseManager::GetInstance().GetConnection();
    con.setAutoCommit(false);

    try
    {
        std::unique_ptr<sql::PreparedStatement> pstmt_session(con.prepareStatement(
            "INSERT INTO GameSessions (HostIPAddress, HostNickname) VALUES (?, ?)"));
        pstmt_session->setString(1, hostIpAddress);
        pstmt_session->setString(2, hostNickname);
        pstmt_session->executeUpdate();

        std::unique_ptr<sql::Statement> stmt(con.createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT LAST_INSERT_ID()"));
        if (!res->next()) {
            throw std::runtime_error("Failed to get last insert ID.");
        }
        newSessionId = res->getUInt(1);

        std::unique_ptr<sql::PreparedStatement> pstmt_player(con.prepareStatement(
            "INSERT INTO SessionPlayers (SessionID, PlayerNickname) VALUES (?, ?)"));
        pstmt_player->setUInt(1, newSessionId);
        pstmt_player->setString(2, hostNickname);
        pstmt_player->executeUpdate();

        con.commit();

        auto nickname_offset = builder.CreateString(hostNickname);
        PacketData::Color player_color(rand() % 256, rand() % 256, rand() % 256);
        auto player_info_obj = PacketData::CreatePlayerInfo(builder, 0, nickname_offset, &player_color, 100, 100, 10, 100, 100, 10);

        std::vector<flatbuffers::Offset<PacketData::PlayerInfo>> players_vector;
        players_vector.push_back(player_info_obj);
        auto players_offset = builder.CreateVector(players_vector);

        auto monsters_offset = builder.CreateVector<flatbuffers::Offset<PacketData::MonsterInfo>>({});
        auto ip_addr_offset = builder.CreateString(hostIpAddress);

        auto session_info_obj = PacketData::CreateSessionInfo(builder, newSessionId, ip_addr_offset, 1, players_offset, monsters_offset, 0, 1);
        auto response_payload = PacketData::CreateS2C_CreateSessionResponse(builder, session_info_obj);

        auto packet = PacketData::CreatePacket(builder, 0, PacketData::PacketPayload_S2C_CreateSessionResponse, response_payload.Union());
        builder.Finish(packet);
    }
    catch (const sql::SQLException& e)
    {
        con.rollback();
        std::cerr << "C2S_CreateSession DB Error: " << e.what() << std::endl;
        return;
    }

    con.setAutoCommit(true);

    boost::asio::async_write(Socket, boost::asio::buffer(builder.GetBufferPointer(), builder.GetSize()),
        boost::bind(&Session::HandleWrite, shared_from_this(),
            boost::asio::placeholders::error));
}

void Session::ProcessJoinSessionRequest(const PacketData::C2S_JoinSession* req)
{
    uint32_t session_id = req->session_id();
    std::string player_nickname = req->player_nickname()->str();

    // TODO: ������ session_id�� Ű��, �ش� ���ǿ� ���� ��� Session* ��ü ����� �����ؾ� �մϴ�.
    // map<uint32_t, vector<shared_ptr<Session>>> GameSessions; �� ���� �ڷᱸ���� �ʿ��մϴ�.

    try
    {
        sql::Connection& con = DatabaseManager::GetInstance().GetConnection();
        // 1. SessionPlayers ���̺� ���ο� �÷��̾� ����
        std::unique_ptr<sql::PreparedStatement> pstmt(con.prepareStatement(
            "INSERT INTO SessionPlayers (SessionID, PlayerNickname) VALUES (?, ?)"));
        pstmt->setUInt(1, session_id);
        pstmt->setString(2, player_nickname);
        pstmt->executeUpdate();

        // 2. DB���� �ֽ� ���� ������ �ٽ� �о�� S2C_UpdateSessionBroadcast ��Ŷ�� ����ϴ�.
        //    (�� ������ ��� ������ ���� �Լ��� ����� ���� �����ϴ�)
        flatbuffers::FlatBufferBuilder builder;
        BuildAndUpdateSession(session_id, builder); // �Ʒ��� ���ǵ� ���� �Լ�

        // 3. TODO: �ش� ������ ��� Ŭ���̾�Ʈ���� �� ��Ŷ�� ��ε�ĳ�����մϴ�.
        // for (auto& client_session : GameSessions[session_id])
        // {
        //     client_session->Send(builder.GetBufferPointer(), builder.GetSize());
        // }
    }
    catch (const sql::SQLException& e)
    {
        std::cerr << "C2S_JoinSession DB Error: " << e.what() << std::endl;
    }
}

// DB���� ���� ������ �о� ��Ŷ�� ����� ���� �Լ�
void Session::BuildAndUpdateSession(uint32_t session_id, flatbuffers::FlatBufferBuilder& builder)
{
    // ... DB���� SessionInfo, ��� PlayerInfo ���� �о���� ���� ...
    // ���������� �Ʒ��� ���� ��Ŷ�� ����ϴ�.
    // auto session_info_obj = PacketData::CreateSessionInfo(builder, ...);
    // auto payload = PacketData::CreateS2C_UpdateSessionBroadcast(builder, session_info_obj);
    // auto packet = PacketData::CreatePacket(builder, 0, PacketData::PacketPayload_S2C_UpdateSessionBroadcast, payload.Union());
    // builder.Finish(packet);
}