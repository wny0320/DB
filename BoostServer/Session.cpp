#include <jdbc/cppconn//driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>

#include "Session.h"
#include "DatabaseManager.h" 

void Session::HandleNicknameCheckRequest(const std::string& nickname)
{
    bool isDuplicate = false;
    try
    {
        // 1. DB Ŀ�ؼ��� �����ɴϴ�.
        sql::Connection& con = DatabaseManager::GetInstance().GetConnection();

        // 2. SQL ������ ������ �����ϱ� ���� PreparedStatement�� ����մϴ�.
        std::unique_ptr<sql::PreparedStatement> pstmt(con.prepareStatement("SELECT COUNT(*) FROM users WHERE nickname = ?"));

        // 3. '?' �ڸ��� ���� �г��� ���� ä�� �ֽ��ϴ�.
        pstmt->setString(1, nickname);

        // 4. ������ �����ϰ� ����� �޽��ϴ�.
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        // 5. ��� Ȯ��: COUNT(*)�� 0���� ũ�� �ߺ��Դϴ�.
        if (res->next() && res->getInt(1) > 0)
        {
            isDuplicate = true;
        }
    }
    catch (sql::SQLException& e)
    {
        // DB ���� ó��
        std::cerr << "DB Query Error: " << e.what() << std::endl;
    }

    // 6. Ŭ���̾�Ʈ���� ���� ��Ŷ�� �����մϴ�.
    // (�� �κ��� ����Ͻô� ��Ŷ �������ݿ� ���� �����մϴ�.)
    // SendNicknameCheckResponse(isDuplicate);
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

        // [����] ���� ���� �Լ��� VerifyPacketBuffer�� ����Ǿ����ϴ�.
        if (PacketData::VerifyPacketBuffer(Verifier))
        {
            // [����] ��Ŷ�� �������� �Լ��� GetPacket���� ����Ǿ����ϴ�.
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
        // ���� �Ϸ�
    }
    else
    {
        std::cerr << "Write Error on Session ID " << Id << ": " << ErrorCode.message() << std::endl;
    }
}

// [����] �Լ��� ���� Ÿ���� Packet*�� ����Ǿ����ϴ�.
void Session::ProcessPacket(const PacketData::Packet* InPacket)
{
    // InPacket�� Ÿ���� ���� PacketData::Packet* �Դϴ�.
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