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
    DatabaseManager(); // private ������

    std::unique_ptr<sql::Connection> connection;

    // RDS ���� ���� (�� �κ��� ����)
    std::string DbHost;
    int DbPort;
    std::string DbUser;
    std::string DbPass;
    std::string DbName;
};
