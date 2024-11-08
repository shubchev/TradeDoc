#pragma once

#include "common.h"
#include "WinSubclass.h"

enum class TextFilter {
  Password = 0,
  Number,
  IntegerNumber,
  PositiveNumber,
  PositiveInteger,
  All,
};

DefineHandle(ITextBoxCb, TextBoxCb);
DefineHandle(ITextBox, TextBox);

class ITextBoxCb {
public:
  virtual ~ITextBoxCb() {}
  // return true to accept the change, false to reject it
  virtual bool onTextChange(ITextBox *, std::wstring &newText) = 0;
};


class ITextBox : public IWinSubclass {
protected:
  TextFilter filter = TextFilter::All;
  mutable std::wstring text;

  bool OnEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

  ITextBox(HWND hParent, int x, int y, int width, int height);

  TextBoxCbRef textCb;

public:
  virtual ~ITextBox() {}

  void setTextEventCb(const TextBoxCb &cb);

  bool setTextFilter(TextFilter filter);

  bool setText(const wchar_t *fmt, ...) override;
  bool setText(const std::wstring &text) override;
  std::wstring getText() const override;

  bool setReadOnly();
  bool setEditable();

  static TextBox create(HWND hParent, int x, int y, int width, int height);
};