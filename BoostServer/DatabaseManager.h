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
    // �̱��� �ν��Ͻ��� �������� static �Լ�
    static DatabaseManager& GetInstance();

    // �����ͺ��̽� ����(����) ��ü�� ��ȯ�ϴ� �Լ�
    mysqlx::Session& GetSession();

    // ���� �� ������ ���� �̱��� ������ ����
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;

private:
    // private ������: �ܺο��� ��ü�� ���� �����ϴ� ���� ����
    DatabaseManager();

private:
    std::unique_ptr<mysqlx::Session> Session; // DB ������ �����ϴ� ����Ʈ ������

    // RDS ���� ����
    std::string DbHost;
    int DbPort;
    std::string DbUser;
    std::string DbPass;
    std::string DbName;
};
