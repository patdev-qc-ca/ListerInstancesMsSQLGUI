#include "ListViewSorter.h"
#pragma warning(disable:6054)

ListViewSorter::ListViewSorter(HWND hList): hList_(hList), sortColumn_(0), ascending_(true){}
void ListViewSorter::OnColumnClick(int column)
{
    if (sortColumn_ == column)
        ascending_ = !ascending_;
    else {
        sortColumn_ = column;
        ascending_ = true;
    }
    ListView_SortItems(hList_, Compare, (LPARAM)this);
}
int CALLBACK ListViewSorter::Compare(LPARAM l1, LPARAM l2, LPARAM sortParam)
{
    ListViewSorter* sorter = (ListViewSorter*)sortParam;
    wchar_t buf1[256], buf2[256];
    LVITEMW item{};
    item.mask = LVIF_TEXT;
    item.iItem = l1;
    item.iSubItem = sorter->sortColumn_;
    item.pszText = buf1;
    item.cchTextMax = 256;
    ListView_GetItem(sorter->hList_, &item);
    item.iItem = l2;
    item.pszText = buf2;
    ListView_GetItem(sorter->hList_, &item);
    int cmp = _wcsicmp(buf1, buf2);
    return sorter->ascending_ ? cmp : -cmp;
}