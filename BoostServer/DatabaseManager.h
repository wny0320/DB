#pragma once

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <mysqlx/xdevapi.h>

#include "PacketData_generated.h"

class DatabaseManager
{
public:
    // 싱글턴 인스턴스를 가져오는 static 함수
    static DatabaseManager& GetInstance();

    // 데이터베이스 세션(연결) 객체를 반환하는 함수
    mysqlx::Session& GetSession();

    // 복사 및 대입을 막아 싱글턴 패턴을 유지
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;

private:
    // private 생성자: 외부에서 객체를 직접 생성하는 것을 막음
    DatabaseManager();

private:
    std::unique_ptr<mysqlx::Session> Session; // DB 세션을 관리하는 스마트 포인터

    // RDS 접속 정보
    std::string DbHost;
    int DbPort;
    std::string DbUser;
    std::string DbPass;
    std::string DbName;
};
