#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>

namespace sql {
    class Connection;
}

class DatabaseManager
{
public:
    static DatabaseManager& GetInstance();

    sql::Connection& GetConnection();

    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;

private:
    DatabaseManager(); // private 생성자

    std::unique_ptr<sql::Connection> connection;

    // RDS 접속 정보 (이 부분은 동일)
    std::string DbHost;
    int DbPort;
    std::string DbUser;
    std::string DbPass;
    std::string DbName;
};
