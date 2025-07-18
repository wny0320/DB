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

// [수정] HandleRead 로직 전체 변경
void Session::HandleRead(const boost::system::error_code& ErrorCode, size_t BytesTransferred)
{
    if (!ErrorCode && BytesTransferred > 0)
    {
        // 1. 새로 받은 데이터를 ReadBuffer의 끝에 추가합니다.
        ReadBuffer.insert(ReadBuffer.end(), DataBuffer, DataBuffer + BytesTransferred);

        const int32_t HeaderSize = 4; // 헤더 사이즈는 4바이트

        // 2. 버퍼에 완전한 패킷이 하나 이상 존재할 수 있으므로, while 루프를 사용합니다.
        while (ReadBuffer.size() >= HeaderSize)
        {
            // 3. 헤더에서 패킷 전체 길이를 읽습니다.
            int32_t PacketLength = 0;
            memcpy(&PacketLength, ReadBuffer.data(), HeaderSize);

            // 4. 패킷이 완전히 도착했는지 확인합니다.
            if (ReadBuffer.size() >= PacketLength)
            {
                // 5. 완전한 패킷의 '실제 데이터' 부분만 ProcessPacket으로 넘겨 처리합니다.
                // (헤더를 제외한 데이터 시작점, 실제 데이터 길이)
                ProcessPacket(ReadBuffer.data() + HeaderSize, PacketLength - HeaderSize);

                // 6. 버퍼에서 처리한 패킷(헤더 포함)을 제거합니다.
                ReadBuffer.erase(ReadBuffer.begin(), ReadBuffer.begin() + PacketLength);
            }
            else
            {
                // 아직 패킷이 다 도착하지 않음. 다음 read를 기다립니다.
                break;
            }
        }

        // 다음 데이터를 계속해서 비동기적으로 기다립니다.
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

// [수정 완료] ProcessPacket() 함수
void Session::ProcessPacket(const uint8_t* data, size_t size)
{
    auto Verifier = flatbuffers::Verifier(data, size);
    if (!PacketData::VerifyPacketBuffer(Verifier))
    {
        // 이 로그는 이제 정말로 데이터가 손상되었을 때만 발생합니다.
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
    // 다른 패킷 타입 처리...
    default:
        std::cout << "Received unknown packet type from Session ID " << Id << std::endl;
        break;
    }
}

// [수정 완료] ProcessCreateSessionRequest() 함수
void Session::ProcessCreateSessionRequest(const PacketData::C2S_CreateSession* req)
{
    std::string hostNickname = req->host_nickname()->str();
    // 클라이언트가 보낸 IP 대신 서버가 저장한 IP를 사용합니다.
    std::string hostIpAddress = this->ClientIP;

    std::cout << "세션 생성 요청 | Nickname: " << hostNickname << ", IP: " << hostIpAddress << std::endl;

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

    // 7. 생성된 응답 패킷을 클라이언트에게 전송 (길이 헤더 추가)

// [추가] 7-1. 패킷의 실제 크기와 헤더(4바이트)를 더한 전체 크기를 계산합니다.
    int32_t packet_size_with_header = builder.GetSize() + 4;

    // [추가] 7-2. 전송할 데이터를 담을 버퍼를 만듭니다.
    //            shared_ptr를 사용하여 비동기 전송이 완료될 때까지 메모리가 안전하게 유지되도록 합니다.
    auto send_buffer = std::make_shared<std::vector<uint8_t>>();
    send_buffer->resize(packet_size_with_header); // 전체 크기만큼 버퍼를 할당합니다.

    // [추가] 7-3. 버퍼의 맨 앞에 4바이트 길이 헤더를 복사합니다.
    memcpy(send_buffer->data(), &packet_size_with_header, 4);

    // [추가] 7-4. 헤더 바로 뒤에 실제 FlatBuffer 데이터를 복사합니다.
    memcpy(send_buffer->data() + 4, builder.GetBufferPointer(), builder.GetSize());

    // [수정] 7-5. 새로 만든 버퍼를 전송합니다.
    boost::asio::async_write(Socket, boost::asio::buffer(*send_buffer),
        // 람다를 사용하여 send_buffer가 전송 완료될 때까지 살아있도록 캡처합니다.
        [self = shared_from_this(), send_buffer](const boost::system::error_code& ec, std::size_t) {
            self->HandleWrite(ec);
        });
}

void Session::ProcessJoinSessionRequest(const PacketData::C2S_JoinSession* req)
{
    uint32_t session_id = req->session_id();
    std::string player_nickname = req->player_nickname()->str();

    // TODO: 서버는 session_id를 키로, 해당 세션에 속한 모든 Session* 객체 목록을 관리해야 합니다.
    // map<uint32_t, vector<shared_ptr<Session>>> GameSessions; 와 같은 자료구조가 필요합니다.

    try
    {
        sql::Connection& con = DatabaseManager::GetInstance().GetConnection();
        // 1. SessionPlayers 테이블에 새로운 플레이어 삽입
        std::unique_ptr<sql::PreparedStatement> pstmt(con.prepareStatement(
            "INSERT INTO SessionPlayers (SessionID, PlayerNickname) VALUES (?, ?)"));
        pstmt->setUInt(1, session_id);
        pstmt->setString(2, player_nickname);
        pstmt->executeUpdate();

        // 2. DB에서 최신 세션 정보를 다시 읽어와 S2C_UpdateSessionBroadcast 패킷을 만듭니다.
        //    (이 로직은 길기 때문에 별도 함수로 만드는 것이 좋습니다)
        flatbuffers::FlatBufferBuilder builder;
        BuildAndUpdateSession(session_id, builder); // 아래에 정의될 헬퍼 함수

        // 3. TODO: 해당 세션의 모든 클라이언트에게 이 패킷을 브로드캐스팅합니다.
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

// DB에서 세션 정보를 읽어 패킷을 만드는 헬퍼 함수
void Session::BuildAndUpdateSession(uint32_t session_id, flatbuffers::FlatBufferBuilder& builder)
{
    // ... DB에서 SessionInfo, 모든 PlayerInfo 등을 읽어오는 로직 ...
    // 최종적으로 아래와 같이 패킷을 만듭니다.
    // auto session_info_obj = PacketData::CreateSessionInfo(builder, ...);
    // auto payload = PacketData::CreateS2C_UpdateSessionBroadcast(builder, session_info_obj);
    // auto packet = PacketData::CreatePacket(builder, 0, PacketData::PacketPayload_S2C_UpdateSessionBroadcast, payload.Union());
    // builder.Finish(packet);
}