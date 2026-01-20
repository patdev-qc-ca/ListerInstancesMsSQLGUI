#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#define WM_USER_SQLINSTANCEFOUND (WM_USER + 100)

class SqlInstanceEnumerator
{
public:
    using Callback = std::function<void(const std::wstring&)>;
    SqlInstanceEnumerator();
    ~SqlInstanceEnumerator();
    void EnumerateAllAsync(Callback cb = nullptr);
    std::vector<std::wstring> GetInstances();
private:
    void EnumerateRegistry();
    void EnumerateSqlBrowser();
    void EnumerateOdbcBrowse();
    void NotifyUI(const std::wstring& name);
    void AddInstance(const std::wstring& name);
private:
    std::vector<std::wstring> instances_;
    std::mutex mutex_;
    Callback callback_;
};