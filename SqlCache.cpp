#include "SqlCache.h"
#include <iostream>

SqlCache::SqlCache(const std::wstring& connStr): connStr_(connStr),hEnv_(NULL),hDbc_(NULL){
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv_);
    if (!SQL_SUCCEEDED(ret)) {
        std::wcerr << L"[SqlCache] SQLAllocHandle(ENV) failed\n";
        hEnv_ = NULL;
        return;
    }
    ret = SQLSetEnvAttr(hEnv_, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(ret)) {
        std::wcerr << L"[SqlCache] SQLSetEnvAttr failed\n";
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv_);
        hEnv_ = NULL;
        return;
    }
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv_, &hDbc_);
    if (!SQL_SUCCEEDED(ret)) {
        std::wcerr << L"[SqlCache] SQLAllocHandle(DBC) failed\n";
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv_);
        hEnv_ = NULL;
        hDbc_ = NULL;
        return;
    }
    ret = SQLDriverConnectW(hDbc_,NULL,(SQLWCHAR*)connStr_.c_str(),SQL_NTS,NULL,0,NULL,SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(ret)) {
        std::wcerr << L"[SqlCache] SQLDriverConnect failed\n";
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc_);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv_);
        hDbc_ = NULL;
        hEnv_ = NULL;
        return;
    }
    std::wcout << L"[SqlCache] Connected to SQL Server successfully\n";
}
SqlCache::~SqlCache()
{
    if (hDbc_) {
        SQLDisconnect(hDbc_);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc_);
    }
    if (hEnv_) {SQLFreeHandle(SQL_HANDLE_ENV, hEnv_);}
}
bool SqlCache::Exists(const std::string& fullName)
{
    SQLHENV  hEnv = NULL;
    SQLHDBC  hDbc = NULL;
    SQLHSTMT hStmt = NULL;
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS)return false;
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    SQLRETURN ret = SQLDriverConnectW(hDbc,NULL,(SQLWCHAR*)connStr_.c_str(),SQL_NTS,NULL,0,NULL,SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "SQLDriverConnect failed\n";
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    std::wstring sql =L"SELECT COUNT(*) FROM GitHubRepos WHERE FullName = ?";
    ret = SQLPrepareW(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "SQLPrepare failed\n";
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    std::wstring wFullName(fullName.begin(), fullName.end());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,wFullName.size(), 0, (SQLPOINTER)wFullName.c_str(), 0, NULL);
    ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "SQLExecute failed\n";
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    SQLLEN count = 0;
    ret = SQLFetch(hStmt);
    if (SQL_SUCCEEDED(ret)) {
        SQLGetData(hStmt, 1, SQL_C_SLONG, &count, 0, NULL);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    return (count > 0);
}
bool SqlCache::Save(const RepoInfo& repo)
{
    SQLHENV hEnv = NULL;
    SQLHDBC hDbc = NULL;
    SQLHSTMT hStmt = NULL;
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) return false;
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    SQLRETURN ret = SQLDriverConnectW(hDbc,NULL,(SQLWCHAR*)connStr_.c_str(),SQL_NTS,NULL,0,NULL,SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "SQLDriverConnect failed\n";
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    std::wstring sql = L"INSERT INTO GitHubRepos (FullName, Url) VALUES (?, ?)";
    ret = SQLPrepareW(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "SQLPrepare failed\n";
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }
    std::wstring fullName = std::wstring(repo.fullName.begin(), repo.fullName.end());
    std::wstring url = std::wstring(repo.url.begin(), repo.url.end());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,fullName.size(), 0, (SQLPOINTER)fullName.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,url.size(), 0, (SQLPOINTER)url.c_str(), 0, NULL);
    ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "SQLExecute failed (duplicate?)\n";
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    return SQL_SUCCEEDED(ret);
}
