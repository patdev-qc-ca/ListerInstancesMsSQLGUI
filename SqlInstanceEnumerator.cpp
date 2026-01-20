#include "SqlInstanceEnumerator.h"
#include <Windows.h>
#include <wtypes.h>
#include <sqlext.h>
#include <winsock.h>
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "ws2_32.lib")
HWND  hwnd_;
HWND InitDialog(HWND notifie);

void SqlInstanceEnumerator::NotifyUI(const std::wstring& name)
{
    PostMessage(hwnd_, WM_USER_SQLINSTANCEFOUND, 0, (LPARAM)new std::wstring(name));
}
void SqlInstanceEnumerator::AddInstance(const std::wstring& name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& s : instances_)
        if (_wcsicmp(s.c_str(), name.c_str()) == 0)return;
    instances_.push_back(name);
    if (callback_)callback_(name);
}
void SqlInstanceEnumerator::EnumerateRegistry()
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Microsoft SQL Server\\Instance Names\\SQL",0, KEY_READ, &hKey) != ERROR_SUCCESS) return;
    DWORD index = 0;
    WCHAR name[256];
    DWORD nameSize = 256;
    while (RegEnumValueW(hKey, index, name, &nameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        AddInstance(name);
        index++;
        nameSize = 256;
    }
    RegCloseKey(hKey);
}
void SqlInstanceEnumerator::EnumerateSqlBrowser()
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) return;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1434);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    const char* msg = "ServerEnum";
    sendto(s, msg, strlen(msg), 0, (sockaddr*)&addr, sizeof(addr));
    char buffer[4096];
    sockaddr_in from{};
    int fromLen = sizeof(from);
    int len = recvfrom(s, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromLen);
    closesocket(s);
    if (len <= 0) return;
    std::string resp(buffer, len);
    size_t pos = 0;
    while ((pos = resp.find("InstanceName", pos)) != std::string::npos)
    {
        size_t start = resp.find(";", pos);
        size_t end = resp.find(";", start + 1);
        std::string inst = resp.substr(start + 1, end - start - 1);
        AddInstance(std::wstring(inst.begin(), inst.end()));
        pos = end;
    }
}
void SqlInstanceEnumerator::EnumerateOdbcBrowse()
{
    SQLHENV hEnv = NULL;
    SQLHDBC hDbc = NULL;
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    SQLWCHAR inStr[] = L"DRIVER={SQL Server};SERVER=?;";
    SQLWCHAR outStr[4096];
    SQLSMALLINT outLen = 0;
    SQLRETURN ret = SQLBrowseConnectW(hDbc, inStr, SQL_NTS,outStr, 4096, &outLen);
    if (ret == SQL_NEED_DATA)
    {
        std::wstring result(outStr);
        size_t pos = result.find(L"SERVER=");
        if (pos != std::wstring::npos)
        {
            pos += 7;
            size_t end = result.find(L";", pos);
            std::wstring list = result.substr(pos, end - pos);
            size_t start = 0;
            while (true)
            {
                size_t comma = list.find(L",", start);
                if (comma == std::wstring::npos) {AddInstance(list.substr(start));break;}
                AddInstance(list.substr(start, comma - start));
                start = comma + 1;
            }
        }
    }
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}
SqlInstanceEnumerator::SqlInstanceEnumerator()
{
}
SqlInstanceEnumerator::~SqlInstanceEnumerator()
{
}
void SqlInstanceEnumerator::EnumerateAllAsync(Callback cb)
{
    callback_ = cb;
    std::thread t1(&SqlInstanceEnumerator::EnumerateRegistry, this);
    std::thread t2(&SqlInstanceEnumerator::EnumerateSqlBrowser, this);
    std::thread t3(&SqlInstanceEnumerator::EnumerateOdbcBrowse, this);
    t1.detach();
    t2.detach();
    t3.detach();
}
std::vector<std::wstring> SqlInstanceEnumerator::GetInstances()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return instances_;
}

HWND InitDialog(HWND notifie){
    hwnd_ = notifie;
    return hwnd_;
}
