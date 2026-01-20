#include "ProgressBarAnimator.h"
#include <commctrl.h>

ProgressBarAnimator::ProgressBarAnimator(HWND hProgress)  : hProgress_(hProgress), hThread_(NULL), running_(false){}
void ProgressBarAnimator::Start()
{
    running_ = true;
    hThread_ = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
}
void ProgressBarAnimator::Stop()
{
    running_ = false;
    if (hThread_) {
        WaitForSingleObject(hThread_, INFINITE);
        CloseHandle(hThread_);
        hThread_ = NULL;
    }
}
DWORD WINAPI ProgressBarAnimator::ThreadProc(LPVOID param)
{
    ((ProgressBarAnimator*)param)->Run();
    return 0;
}
void ProgressBarAnimator::Run()
{
    int pos = 0;
    while (running_) {
        SendMessage(hProgress_, PBM_SETPOS, pos, 0);
        pos = (pos + 5) % 100;
        Sleep(50);
    }
}