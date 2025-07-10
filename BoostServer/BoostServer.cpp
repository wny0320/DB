#include <boost/thread/thread.hpp>

#include "BoostServer.h"

BoostServer::BoostServer(boost::asio::io_context& io_context, short port)
    : io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    StartAccept();
}

void BoostServer::StartAccept() {
    // ���ο� ���� ��ü ����
    uint32_t new_id;
    {
        boost::lock_guard<boost::mutex> lock(mtx_);
        new_id = ++id_count_;
    }
    boost::shared_ptr<Session> new_session(new Session(io_context_, new_id));

    // �񵿱������� Ŭ���̾�Ʈ ���� ���
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&BoostServer::HandleAccept, this, new_session,
            boost::asio::placeholders::error));
}

void BoostServer::HandleAccept(boost::shared_ptr<Session> new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->Start(); // ���������� ���ӵǸ� ���� ����
    }
    else {
        std::cerr << "Accept Error: " << error.message() << std::endl;
    }

    StartAccept(); // ���� Ŭ���̾�Ʈ ������ ���� �ٽ� ���
}