#include <boost/thread/thread.hpp>
#include "BoostServer.h"

BoostServer::BoostServer(boost::asio::io_context& IoContext, short Port)
    : IoContext(IoContext),
    Acceptor(IoContext, tcp::endpoint(tcp::v4(), Port))
{
    StartAccept();
}

// �񵿱� ���� ��� ���� �Լ� ����
void BoostServer::StartAccept()
{
    // �����忡 �����ϰ� ���ο� ������ ���� ID�� �����մϴ�.
    uint32_t NewId;
    {
        boost::lock_guard<boost::mutex> Lock(Mutex);
        NewId = ++IdCount;
    }

    // ���ο� Ŭ���̾�Ʈ�� ����� ���� ��ü�� �����մϴ�.
    boost::shared_ptr<Session> NewSession(new Session(IoContext, NewId));

    // Acceptor�� ���� �񵿱������� Ŭ���̾�Ʈ ������ ��ٸ��ϴ�.
    // ������ �̷������ HandleAccept �Լ��� ȣ��˴ϴ�.
    Acceptor.async_accept(NewSession->GetSocket(),
        boost::bind(&BoostServer::HandleAccept, this, NewSession,
            boost::asio::placeholders::error));
}

// �񵿱� ���� �Ϸ� �ݹ� �Լ� ����
void BoostServer::HandleAccept(boost::shared_ptr<Session> NewSession, const boost::system::error_code& ErrorCode)
{
    if (!ErrorCode)
    {
        // 1. ������ ���� ��ü�� �����ɴϴ�.
        tcp::socket& socket = NewSession->GetSocket();

        // 2. ���Ͽ� ����� ����(Ŭ���̾�Ʈ)�� ��������Ʈ(IP�ּ�+��Ʈ) ������ �����ɴϴ�.
        boost::system::error_code ec;
        tcp::endpoint remote_ep = socket.remote_endpoint(ec);

        if (!ec)
        {
            // 3. ���������� ������ �����Դٸ�, Session ��ü�� IP �ּҸ� �����մϴ�.
            std::string client_ip = remote_ep.address().to_string();
            NewSession->SetClientIP(client_ip);

            std::cout << "Ŭ���̾�Ʈ ���� ����. Session ID: " << NewSession->GetId() << ", IP: " << client_ip << std::endl;
        }
        else
        {
            std::cerr << "Ŭ���̾�Ʈ IP �ּ� ȹ�� ����: " << ec.message() << std::endl;
        }

        // 4. ������ �񵿱� �б� ����
        NewSession->Start();
    }
    else
    {
        std::cerr << "Accept Error: " << ErrorCode.message() << std::endl;
    }

    // ���� Ŭ���̾�Ʈ�� ������ ��ٸ��ϴ�.
    StartAccept();
}