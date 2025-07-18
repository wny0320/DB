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

void BoostServer::HandleAccept(boost::shared_ptr<Session> NewSession, const boost::system::error_code& ErrorCode)
{
    // [디버그 로그 1] HandleAccept 함수가 호출되었는지 확인
    std::cout << "[DEBUG] HandleAccept 진입." << std::endl;

    if (!ErrorCode)
    {
        // [디버그 로그 2] 접속 에러가 없음을 확인
        std::cout << "[DEBUG] 접속 성공 (ErrorCode 없음). IP 주소 확인 시작..." << std::endl;

        // 클라이언트의 IP 주소를 획득하고 Session 객체에 설정합니다.
        try
        {
            tcp::socket& socket = NewSession->GetSocket();
            boost::system::error_code ec;
            tcp::endpoint remote_ep = socket.remote_endpoint(ec);

            if (!ec)
            {
                std::string client_ip = remote_ep.address().to_string();
                NewSession->SetClientIP(client_ip);
                std::cout << "[DEBUG] IP 주소 획득 성공: " << client_ip << std::endl;
            }
            else
            {
                // IP 주소 획득에 실패하더라도 세션은 시작해야 합니다.
                NewSession->SetClientIP("0.0.0.0"); // 실패 시 기본값 설정
                std::cerr << "[DEBUG] IP 주소 획득 실패: " << ec.message() << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[DEBUG] IP 주소 획득 중 예외 발생: " << e.what() << std::endl;
        }

        // [디버그 로그 3] Session::Start() 호출 직전인지 확인
        std::cout << "[DEBUG] Session::Start() 호출 준비..." << std::endl;
        NewSession->Start();
    }
    else
    {
        // 접속 자체에 실패한 경우 (예: 서버 리소스 부족)
        std::cerr << "Accept Error: " << ErrorCode.message() << std::endl;
    }

    // 다음 클라이언트의 접속을 받기 위해 다시 대기 상태로 들어갑니다.
    StartAccept();
}