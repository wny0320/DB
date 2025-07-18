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

void BoostServer::HandleAccept(boost::shared_ptr<Session> NewSession, const boost::system::error_code& ErrorCode)
{
    // [����� �α� 1] HandleAccept �Լ��� ȣ��Ǿ����� Ȯ��
    std::cout << "[DEBUG] HandleAccept ����." << std::endl;

    if (!ErrorCode)
    {
        // [����� �α� 2] ���� ������ ������ Ȯ��
        std::cout << "[DEBUG] ���� ���� (ErrorCode ����). IP �ּ� Ȯ�� ����..." << std::endl;

        // Ŭ���̾�Ʈ�� IP �ּҸ� ȹ���ϰ� Session ��ü�� �����մϴ�.
        try
        {
            tcp::socket& socket = NewSession->GetSocket();
            boost::system::error_code ec;
            tcp::endpoint remote_ep = socket.remote_endpoint(ec);

            if (!ec)
            {
                std::string client_ip = remote_ep.address().to_string();
                NewSession->SetClientIP(client_ip);
                std::cout << "[DEBUG] IP �ּ� ȹ�� ����: " << client_ip << std::endl;
            }
            else
            {
                // IP �ּ� ȹ�濡 �����ϴ��� ������ �����ؾ� �մϴ�.
                NewSession->SetClientIP("0.0.0.0"); // ���� �� �⺻�� ����
                std::cerr << "[DEBUG] IP �ּ� ȹ�� ����: " << ec.message() << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[DEBUG] IP �ּ� ȹ�� �� ���� �߻�: " << e.what() << std::endl;
        }

        // [����� �α� 3] Session::Start() ȣ�� �������� Ȯ��
        std::cout << "[DEBUG] Session::Start() ȣ�� �غ�..." << std::endl;
        NewSession->Start();
    }
    else
    {
        // ���� ��ü�� ������ ��� (��: ���� ���ҽ� ����)
        std::cerr << "Accept Error: " << ErrorCode.message() << std::endl;
    }

    // ���� Ŭ���̾�Ʈ�� ������ �ޱ� ���� �ٽ� ��� ���·� ���ϴ�.
    StartAccept();
}