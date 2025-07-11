#include "DatabaseManager.h"
#include <cstdlib>

DatabaseManager& DatabaseManager::GetInstance()
{
    static DatabaseManager Instance; // ���α׷� ���� �� �� ���� ������
    return Instance;
}

// private ������ ����
DatabaseManager::DatabaseManager()
{
    try
    {
        // 1. C++ ǥ�� ���̺귯�� <cstdlib>�� getenv �Լ��� ȯ�� ���� �б�
        const char* HostEnv = std::getenv("DB_HOST");
        const char* PortEnv = std::getenv("DB_PORT");
        const char* UserEnv = std::getenv("DB_USER");
        const char* PassEnv = std::getenv("DB_PASS");
        const char* NameEnv = std::getenv("DB_NAME");
        
        // 2. ȯ�� ������ ����� �����Ǿ����� Ȯ��
        if (!HostEnv || !PortEnv || !UserEnv || !PassEnv || !NameEnv)
        {
            throw std::runtime_error("Database environment variables are not set!");
        }

        // 3. �о�� ȯ�� ������ ��� ������ ����
        DbHost = HostEnv;
        DbPort = std::stoi(PortEnv); // ���ڿ��� ������ ��ȯ
        DbUser = UserEnv;
        DbPass = PassEnv;
        DbName = NameEnv;

        std::cout << "Attempting to connect to database at " << DbHost << std::endl;

        // 4. ����� ������ RDS�� ����
        Session = std::make_unique<mysqlx::Session>(mysqlx::SessionOption::HOST, DbHost,
            mysqlx::SessionOption::PORT, DbPort,
            mysqlx::SessionOption::USER, DbUser,
            mysqlx::SessionOption::PWD, DbPass,
            mysqlx::SessionOption::DB, DbName);

        std::cout << "Database connection successful!" << std::endl;
    }
    catch (const mysqlx::Error& e)
    {
        // MySQL Connector���� �߻��� ���� ó��
        std::cerr << "Database connection failed: " << e.what() << std::endl;
        throw; // ���� ������ �ߴܽ�Ű�� ���� ���ܸ� �ٽ� ����
    }
    catch (const std::exception& e)
    {
        // �� �� �ٸ� ���� ó�� (e.g., std::stoi ����, ȯ�� ���� ����)
        std::cerr << "An error occurred during database setup: " << e.what() << std::endl;
        throw;
    }
}

// �����ͺ��̽� ���� ��ȯ
mysqlx::Session& DatabaseManager::GetSession()
{
    return *Session;
}
