#include <boost/thread/thread.hpp>
#include "BoostServer.h"

BoostServer::BoostServer(boost::asio::io_context& IoContext, short Port)
    : IoContext(IoContext),
    Acceptor(IoContext, tcp::endpoint(tcp::v4(), Port))
{
    StartAccept();
}

// 비동기 접속 대기 시작 함수 구현
void BoostServer::StartAccept()
{
    // 스레드에 안전하게 새로운 세션의 고유 ID를 생성합니다.
    uint32_t NewId;
    {
        boost::lock_guard<boost::mutex> Lock(Mutex);
        NewId = ++IdCount;
    }

    // 새로운 클라이언트를 담당할 세션 객체를 생성합니다.
    boost::shared_ptr<Session> NewSession(new Session(IoContext, NewId));

    // Acceptor를 통해 비동기적으로 클라이언트 접속을 기다립니다.
    // 접속이 이루어지면 HandleAccept 함수가 호출됩니다.
    Acceptor.async_accept(NewSession->GetSocket(),
        boost::bind(&BoostServer::HandleAccept, this, NewSession,
            boost::asio::placeholders::error));
}

// 비동기 접속 완료 콜백 함수 구현
void BoostServer::HandleAccept(boost::shared_ptr<Session> NewSession, const boost::system::error_code& ErrorCode)
{
    if (!ErrorCode)
    {
        // 1. 세션의 소켓 객체를 가져옵니다.
        tcp::socket& socket = NewSession->GetSocket();

        // 2. 소켓에 연결된 상대방(클라이언트)의 엔드포인트(IP주소+포트) 정보를 가져옵니다.
        boost::system::error_code ec;
        tcp::endpoint remote_ep = socket.remote_endpoint(ec);

        if (!ec)
        {
            // 3. 성공적으로 정보를 가져왔다면, Session 객체에 IP 주소를 설정합니다.
            std::string client_ip = remote_ep.address().to_string();
            NewSession->SetClientIP(client_ip);

            std::cout << "클라이언트 접속 성공. Session ID: " << NewSession->GetId() << ", IP: " << client_ip << std::endl;
        }
        else
        {
            std::cerr << "클라이언트 IP 주소 획득 실패: " << ec.message() << std::endl;
        }

        // 4. 세션의 비동기 읽기 시작
        NewSession->Start();
    }
    else
    {
        std::cerr << "Accept Error: " << ErrorCode.message() << std::endl;
    }

    // 다음 클라이언트의 접속을 기다립니다.
    StartAccept();
}