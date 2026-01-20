#pragma once
#include <windows.h>

class ProgressBarAnimator
{
public:
    ProgressBarAnimator(HWND hProgress);
    void Start();
    void Stop();
private:
    static DWORD WINAPI ThreadProc(LPVOID param);
    void Run();
private:
    HWND hProgress_;
    HANDLE hThread_;
    bool running_;
};