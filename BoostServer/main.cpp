// main.cpp
#include <boost/thread/thread.hpp>
#include "BoostServer.h"

#define SERVER_PORT 30303
#define THREAD_POOL_SIZE 4 // CPU �ھ� ���� �°� ����

int main() {
    try {
        boost::asio::io_context io_context;
        BoostServer Server(io_context, SERVER_PORT);

        // ������ Ǯ ����
        std::vector<boost::shared_ptr<boost::thread>> threads;
        for (std::size_t i = 0; i < THREAD_POOL_SIZE; ++i) {
            boost::shared_ptr<boost::thread> thread(new boost::thread(
                boost::bind(&boost::asio::io_context::run, &io_context)));
            threads.push_back(thread);
        }

        std::cout << "Server started on port " << SERVER_PORT << " with " << THREAD_POOL_SIZE << " threads." << std::endl;

        // ��� �����尡 ����� ������ ���
        for (std::size_t i = 0; i < threads.size(); ++i) {
            threads[i]->join();
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}