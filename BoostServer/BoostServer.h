#pragma once

#include <boost/thread/mutex.hpp>
#include "Session.h"

class BoostServer {
public:
    BoostServer(boost::asio::io_context& io_context, short port);

private:
    void StartAccept();
    void HandleAccept(boost::shared_ptr<Session> new_session, const boost::system::error_code& error);

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    boost::mutex mtx_;
    uint32_t id_count_ = 0;
};