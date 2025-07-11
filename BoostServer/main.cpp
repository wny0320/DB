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
        // 가장 먼저 데이터베이스 연결을 시도
        // 연결 실패 시 여기서 예외가 발생하여 서버가 시작되지 않음
        DatabaseManager::GetInstance();

        boost::asio::io_context IoContext;

        BoostServer GameServer(IoContext, SERVER_PORT);

        // 비동기 작업들을 병렬로 처리하기 위한 스레드 풀을 생성합니다.
        std::vector<boost::shared_ptr<boost::thread>> ThreadPool;
        for (std::size_t i = 0; i < THREAD_POOL_SIZE; ++i)
        {
            boost::shared_ptr<boost::thread> Thread(new boost::thread(
                boost::bind(&boost::asio::io_context::run, &IoContext)));
            ThreadPool.push_back(Thread);
        }

        std::cout << "Server started on port " << SERVER_PORT << " with " << THREAD_POOL_SIZE << " threads." << std::endl;

        // 생성된 모든 스레드가 작업을 마칠 때까지 기다립니다.
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
