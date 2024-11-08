#pragma once

#include "common.h"
#include "WinSubclass.h"

DefineHandle(ILabel, Label);
class ILabel : public IWinSubclass {
protected:
  ILabel(HWND parent, int x, int y, int width, int height);

public:
  virtual ~ILabel() {}

  bool setText(const wchar_t *fmt, ...) override;
  bool setText(const std::wstring &text) override;

  static int getTextLength(HWND hWnd, const std::wstring &text);
  static RECT getTextRect(HWND hWnd, const std::wstring &text);

  static Label create(HWND hParent, int x, int y, int width, int height);
};