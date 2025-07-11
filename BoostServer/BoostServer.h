#pragma once

#define NOMINMAX

#include "Session.h"
#include "PacketData_generated.h"

class BoostServer
{
public:
    // 생성자: I/O 서비스와 포트 번호를 받아 서버를 초기화합니다.
    BoostServer(boost::asio::io_context& IoContext, short Port);

private:
    // 클라이언트의 접속을 비동기적으로 기다리는 작업을 시작합니다.
    void StartAccept();

    // 클라이언트 접속이 완료되면 호출될 콜백 함수입니다.
    void HandleAccept(boost::shared_ptr<Session> NewSession, const boost::system::error_code& ErrorCode);

private:
    boost::asio::io_context& IoContext; // I/O 작업을 처리할 컨텍스트
    tcp::acceptor Acceptor;             // 클라이언트 접속을 수락하는 객체
    boost::mutex Mutex;                 // 세션 ID 생성을 위한 뮤텍스
    uint32_t IdCount = 0;               // 세션 ID 카운터
};
