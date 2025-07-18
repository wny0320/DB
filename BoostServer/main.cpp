#define NOMINMAX

#include <boost/thread/thread.hpp>
#include "BoostServer.h"
#include "DatabaseManager.h"

// ������ FlatBuffer ��� ����
#include "PacketData_generated.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define SERVER_PORT 32124
#define THREAD_POOL_SIZE 4

int main()
{
#ifdef _WIN32
    // Windows �ܼ��� ��� ���ڵ��� UTF-8�� �����մϴ�.
    SetConsoleOutputCP(CP_UTF8);
    // Windows �ܼ��� �Է� ���ڵ��� UTF-8�� �����մϴ�. (���� ����)
    // SetConsoleCP(CP_UTF8);
#endif
    try
    {
        // 1. DB ������ ���� �õ��մϴ�.
        DatabaseManager::GetInstance();
        std::cout << "[INFO] Database connection successful." << std::endl;

        // 2. �񵿱� I/O�� ó���� io_context ��ü�� �����մϴ�.
        boost::asio::io_context IoContext;

        // 3. ���� ��ü�� �����մϴ�. �� ������ async_accept�� ��û�˴ϴ�.
        BoostServer GameServer(IoContext, SERVER_PORT);
        std::cout << "[INFO] Server object created, acceptor is open." << std::endl;


        // 4. --- ���� �߿��� �κ� ---
        // io_context.run()�� ������ ������ Ǯ�� �����մϴ�.
        // �� ��������� ������ �񵿱� �۾��� ó���մϴ�.
        std::vector<boost::shared_ptr<boost::thread>> ThreadPool;
        for (std::size_t i = 0; i < THREAD_POOL_SIZE; ++i)
        {
            boost::shared_ptr<boost::thread> Thread(new boost::thread(
                boost::bind(&boost::asio::io_context::run, &IoContext)));
            ThreadPool.push_back(Thread);
        }

        std::cout << "=================================================" << std::endl;
        std::cout << "Server started on port " << SERVER_PORT << " with " << THREAD_POOL_SIZE << " threads." << std::endl;
        std::cout << "Waiting for clients..." << std::endl;
        std::cout << "=================================================" << std::endl;

        // 5. ������ ��� �����尡 �۾��� ��ĥ ������ ���� ������� ��ٸ��ϴ�.
        // (������ ������� �ʵ��� ����Ƶδ� ����)
        for (std::size_t i = 0; i < ThreadPool.size(); ++i)
        {
            ThreadPool[i]->join();
        }
    }
    catch (const std::exception& e)
    {
        // ���� ���� �������� ���� �߻� �� (��: ��Ʈ �̹� ��� ��)
        std::cerr << "[FATAL ERROR] Exception during server startup: " << e.what() << std::endl;
    }

    return 0;
}
