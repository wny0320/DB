#include "DatabaseManager.h"
#include <cstdlib>

DatabaseManager& DatabaseManager::GetInstance()
{
    static DatabaseManager Instance; // 프로그램 시작 시 한 번만 생성됨
    return Instance;
}

// private 생성자 구현
DatabaseManager::DatabaseManager()
{
    try
    {
        // 1. C++ 표준 라이브러리 <cstdlib>의 getenv 함수로 환경 변수 읽기
        const char* HostEnv = std::getenv("DB_HOST");
        const char* PortEnv = std::getenv("DB_PORT");
        const char* UserEnv = std::getenv("DB_USER");
        const char* PassEnv = std::getenv("DB_PASS");
        const char* NameEnv = std::getenv("DB_NAME");
        
        // 2. 환경 변수가 제대로 설정되었는지 확인
        if (!HostEnv || !PortEnv || !UserEnv || !PassEnv || !NameEnv)
        {
            throw std::runtime_error("Database environment variables are not set!");
        }

        // 3. 읽어온 환경 변수를 멤버 변수에 저장
        DbHost = HostEnv;
        DbPort = std::stoi(PortEnv); // 문자열을 정수로 변환
        DbUser = UserEnv;
        DbPass = PassEnv;
        DbName = NameEnv;

        std::cout << "Attempting to connect to database at " << DbHost << std::endl;

        // 4. 저장된 정보로 RDS에 연결
        Session = std::make_unique<mysqlx::Session>(mysqlx::SessionOption::HOST, DbHost,
            mysqlx::SessionOption::PORT, DbPort,
            mysqlx::SessionOption::USER, DbUser,
            mysqlx::SessionOption::PWD, DbPass,
            mysqlx::SessionOption::DB, DbName);

        std::cout << "Database connection successful!" << std::endl;
    }
    catch (const mysqlx::Error& e)
    {
        // MySQL Connector에서 발생한 에러 처리
        std::cerr << "Database connection failed: " << e.what() << std::endl;
        throw; // 서버 시작을 중단시키기 위해 예외를 다시 던짐
    }
    catch (const std::exception& e)
    {
        // 그 외 다른 예외 처리 (e.g., std::stoi 실패, 환경 변수 누락)
        std::cerr << "An error occurred during database setup: " << e.what() << std::endl;
        throw;
    }
}

// 데이터베이스 세션 반환
mysqlx::Session& DatabaseManager::GetSession()
{
    return *Session;
}
