#define _CRT_SECURE_NO_WARNINGS

#include "DatabaseManager.h"
#include <cstdlib>
#include <memory>

#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/connection.h>

DatabaseManager::DatabaseManager()
{
    try
    {
        DbHost = getenv("DB_HOST");
        DbPort = std::stoi(getenv("DB_PORT"));
        DbUser = getenv("DB_USER");
        DbPass = getenv("DB_PASS");
        DbName = getenv("DB_NAME");

        std::cout << "Attempting to connect to database at " << DbHost << ":" << DbPort << std::endl;

        sql::Driver* driver = get_driver_instance();

        std::string connection_url = "tcp://" + DbHost + ":" + std::to_string(DbPort);

        connection.reset(driver->connect(connection_url, DbUser, DbPass));

        connection->setSchema(DbName);

        std::cout << "Database connection successful!" << std::endl;

    }
    catch (const sql::SQLException& e)
    {
        std::cerr << "Database connection failed (SQLException): " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << ")" << std::endl;
        throw;
    }
}

DatabaseManager& DatabaseManager::GetInstance()
{
    static DatabaseManager instance;
    return instance;
}

sql::Connection& DatabaseManager::GetConnection()
{
    if (!connection) {
        throw std::runtime_error("Database is not connected.");
    }
    return *connection;
}
