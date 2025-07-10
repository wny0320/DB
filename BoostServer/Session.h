#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>

#include "PacketData_generated.h"

using boost::asio::ip::tcp;

class Session : public boost::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& io_context, uint32_t id) : socket_(io_context), id_(id) {}

	tcp::socket& socket()
	{
		return socket_;
	}

	void Start();
private:
	void HandleRead(const boost::system::error_code& error, size_t bytes_transferred);
	void HandleWrite(const boost::system::error_code& error);
	void ProcessPacket(const PacketData::PacketData* packet);

	tcp::socket socket_;
	uint32_t id_;
	enum { max_length = 1024 };
	char data_[max_length];
};