#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include "RepoInfo.h"

class SqlCache {
public:
    SqlCache(const std::wstring& connStr);
    ~SqlCache();
    bool Save(const RepoInfo& repo);
    bool Exists(const std::string& fullName);

private:
    std::wstring connStr_;
    SQLHANDLE hEnv_;
    SQLHANDLE hDbc_;
};