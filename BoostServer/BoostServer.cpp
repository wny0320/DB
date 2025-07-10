#include <boost/thread/thread.hpp>

#include "BoostServer.h"

BoostServer::BoostServer(boost::asio::io_context& io_context, short port)
    : io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    StartAccept();
}

void BoostServer::StartAccept() {
    // 새로운 세션 객체 생성
    uint32_t new_id;
    {
        boost::lock_guard<boost::mutex> lock(mtx_);
        new_id = ++id_count_;
    }
    boost::shared_ptr<Session> new_session(new Session(io_context_, new_id));

    // 비동기적으로 클라이언트 접속 대기
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&BoostServer::HandleAccept, this, new_session,
            boost::asio::placeholders::error));
}

void BoostServer::HandleAccept(boost::shared_ptr<Session> new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->Start(); // 성공적으로 접속되면 세션 시작
    }
    else {
        std::cerr << "Accept Error: " << error.message() << std::endl;
    }

    StartAccept(); // 다음 클라이언트 접속을 위해 다시 대기
}