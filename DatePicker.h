#pragma once

#include "common.h"
#include "WinSubclass.h"

DefineHandle(IDatePicker, DatePicker);
class IDatePicker : public IWinSubclass {
protected:
  bool OnNotify(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

  IDatePicker(HWND hParent, int x, int y, int width, int height);

public:
  virtual ~IDatePicker() {}

  SYSTEMTIME getValue() const;
  bool setValue(const SYSTEMTIME &time);

  static DatePicker create(HWND hParent, int x, int y, int width, int height);
};