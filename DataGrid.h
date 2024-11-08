#pragma once

#include "common.h"
#include "WinSubclass.h"

DefineHandle(IDataGrid, DataGrid);
class IDataGrid : public IWinSubclass {
protected:
  IDataGrid(HWND parent, int x, int y, int width, int height);

  bool OnNotify(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

  std::vector<std::wstring> columnName;

public:
  virtual ~IDataGrid() {}

  bool addColumn(const std::wstring &name, int width);
  bool addItem();

  static DataGrid create(HWND hParent, int x, int y, int width, int height);
};