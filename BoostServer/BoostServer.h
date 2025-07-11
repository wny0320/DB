#pragma once

#define NOMINMAX

#include "Session.h"
#include "PacketData_generated.h"

class BoostServer
{
public:
    // ������: I/O ���񽺿� ��Ʈ ��ȣ�� �޾� ������ �ʱ�ȭ�մϴ�.
    BoostServer(boost::asio::io_context& IoContext, short Port);

private:
    // Ŭ���̾�Ʈ�� ������ �񵿱������� ��ٸ��� �۾��� �����մϴ�.
    void StartAccept();

    // Ŭ���̾�Ʈ ������ �Ϸ�Ǹ� ȣ��� �ݹ� �Լ��Դϴ�.
    void HandleAccept(boost::shared_ptr<Session> NewSession, const boost::system::error_code& ErrorCode);

private:
    boost::asio::io_context& IoContext; // I/O �۾��� ó���� ���ؽ�Ʈ
    tcp::acceptor Acceptor;             // Ŭ���̾�Ʈ ������ �����ϴ� ��ü
    boost::mutex Mutex;                 // ���� ID ������ ���� ���ؽ�
    uint32_t IdCount = 0;               // ���� ID ī����
};
