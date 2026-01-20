#pragma once
#pragma once
#include <windows.h>
#include <commctrl.h>
#include <string>

class ListViewSorter
{
public:
    ListViewSorter(HWND hList);
    void OnColumnClick(int column);
    static int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
private:
    HWND hList_;
    int sortColumn_;
    bool ascending_;
};
