#define NOMINMAX
#define _WIN32_WINNT 0x0A00

#include <boost/thread/thread.hpp>
#include "BoostServer.h"
#include "DatabaseManager.h"

#include "PacketData_generated.h"

#define SERVER_PORT 30303
#define THREAD_POOL_SIZE 4

int main()
{
    try
    {
        // ���� ���� �����ͺ��̽� ������ �õ�
        // ���� ���� �� ���⼭ ���ܰ� �߻��Ͽ� ������ ���۵��� ����
        DatabaseManager::GetInstance();

        boost::asio::io_context IoContext;

        BoostServer GameServer(IoContext, SERVER_PORT);

        // �񵿱� �۾����� ���ķ� ó���ϱ� ���� ������ Ǯ�� �����մϴ�.
        std::vector<boost::shared_ptr<boost::thread>> ThreadPool;
        for (std::size_t i = 0; i < THREAD_POOL_SIZE; ++i)
        {
            boost::shared_ptr<boost::thread> Thread(new boost::thread(
                boost::bind(&boost::asio::io_context::run, &IoContext)));
            ThreadPool.push_back(Thread);
        }

        std::cout << "Server started on port " << SERVER_PORT << " with " << THREAD_POOL_SIZE << " threads." << std::endl;

        // ������ ��� �����尡 �۾��� ��ĥ ������ ��ٸ��ϴ�.
        for (std::size_t i = 0; i < ThreadPool.size(); ++i)
        {
            ThreadPool[i]->join();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
