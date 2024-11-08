#include "DataGrid.h"
#include <CommCtrl.h>

class DataGridImpl : public IDataGrid {
public:
  DataGridImpl(HWND parent, int x, int y, int width, int height)
    : IDataGrid(parent, x, y, width, height) {
  }
};

DataGrid IDataGrid::create(HWND hParent, int x, int y, int width, int height) {
  return MakeHandle(DataGridImpl, hParent, x, y, width, height);
}

IDataGrid::IDataGrid(HWND parent, int x, int y, int width, int height) {
  if (!createWindow(parent, WC_LISTVIEW,
                    WS_VISIBLE | WS_BORDER | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
                    x, y, width, height)) {
    throw std::runtime_error("");
  }
}

bool IDataGrid::addColumn(const std::wstring &name, int width) {
  int idx = (int)columnName.size();
  columnName.push_back(name);

  LVCOLUMN ci;
  ci.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  ci.iSubItem = idx;
  ci.pszText = columnName.back().data();
  ci.cx = width;
  ci.fmt = LVCFMT_LEFT;
  return ListView_InsertColumn(hWnd, idx, &ci) == idx;
}

bool IDataGrid::addItem() {
  std::wstring txt;
  LVITEM ii;
  memset(&ii, 0, sizeof(ii));
  ii.mask = LVIF_TEXT | LVIF_STATE;
  ii.cchTextMax = 6;
  static int ttt = 0;
  txt = std::to_wstring(ttt);
  ii.pszText = txt.data();
  ii.iItem = ttt;
  auto ret = ListView_InsertItem(hWnd, &ii);
  ttt++;

  ListView_EditLabel(hWnd, ret);

  return true;
}

bool IDataGrid::OnNotify(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  NMLVDISPINFO *plvdi;

  switch (((LPNMHDR)lParam)->code) {
    case LVN_GETDISPINFO:

      plvdi = (NMLVDISPINFO *)lParam;

      plvdi->item.iItem += 0;

      break;

  }
  return false;
}