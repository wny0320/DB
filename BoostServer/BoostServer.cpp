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
        NewSession->Start(); // 접속 성공 시, 세션 로직 시작
    }
    else
    {
        std::cerr << "Accept Error: " << ErrorCode.message() << std::endl;
    }

    // 다음 클라이언트의 접속을 받기 위해 다시 대기 상태로 들어갑니다.
    StartAccept();
}
