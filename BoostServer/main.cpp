#define NOMINMAX

#include <boost/thread/thread.hpp>
#include "BoostServer.h"
#include "DatabaseManager.h"

// 생성된 FlatBuffer 헤더 파일
#include "PacketData_generated.h"

#define SERVER_PORT 32124
#define THREAD_POOL_SIZE 4

int main()
{
    try
    {
        // 1. DB 연결을 먼저 시도합니다.
        DatabaseManager::GetInstance();
        std::cout << "[INFO] Database connection successful." << std::endl;

        // 2. 비동기 I/O를 처리할 io_context 객체를 생성합니다.
        boost::asio::io_context IoContext;

        // 3. 서버 객체를 생성합니다. 이 시점에 async_accept가 요청됩니다.
        BoostServer GameServer(IoContext, SERVER_PORT);
        std::cout << "[INFO] Server object created, acceptor is open." << std::endl;


        // 4. --- 가장 중요한 부분 ---
        // io_context.run()을 실행할 스레드 풀을 생성합니다.
        // 이 스레드들이 실제로 비동기 작업을 처리합니다.
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

        // 5. 생성된 모든 스레드가 작업을 마칠 때까지 메인 스레드는 기다립니다.
        // (서버가 종료되지 않도록 붙잡아두는 역할)
        for (std::size_t i = 0; i < ThreadPool.size(); ++i)
        {
            ThreadPool[i]->join();
        }
    }
    catch (const std::exception& e)
    {
        // 서버 시작 과정에서 오류 발생 시 (예: 포트 이미 사용 중)
        std::cerr << "[FATAL ERROR] Exception during server startup: " << e.what() << std::endl;
    }

    return 0;
}
