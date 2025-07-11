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
        NewSession->Start(); // ���� ���� ��, ���� ���� ����
    }
    else
    {
        std::cerr << "Accept Error: " << ErrorCode.message() << std::endl;
    }

    // ���� Ŭ���̾�Ʈ�� ������ �ޱ� ���� �ٽ� ��� ���·� ���ϴ�.
    StartAccept();
}
