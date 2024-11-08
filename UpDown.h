#pragma once

#include "TextBox.h"

DefineHandle(IUpDownPicker, UpDownPicker);
DefineHandle(IUpDownPickerCb, UpDownPickerCb);
class IUpDownPickerCb {
public:
  virtual ~IUpDownPickerCb() {}
  virtual bool onUpDownChange(IUpDownPicker *win, int64_t &newVal) { return true; }
};


class IUpDownPicker : public IWinSubclass, public IWinEventCb, public ITextBoxCb {
protected:
  int udSize = 0;
  TextBox text;
  std::wstring fmt;
  int64_t minV = 0;
  int64_t maxV = 0;
  int64_t curV = 0;
  bool arrows = true;

  bool onTextChange(ITextBox *tb, std::wstring &newText) override;
  bool OnNotify(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
  bool OnWinEvent(IWinSubclass *win, UINT msg, WPARAM wParam, LPARAM lParam) override;

  IUpDownPicker(HWND hParent, int x, int y, int width, int height);

  UpDownPickerCbRef udCb;

public:
  virtual ~IUpDownPicker() {}

  HWND getTextFieldHandle() const { return text->getHandle(); }

  bool setText(const std::wstring &fmt) override;
  bool setText(const wchar_t *fmt, ...) override;
  std::wstring getText() const override;
  bool show() override;
  bool hide() override;
  bool setPos(int x, int y) override;
  bool setX(int x) override;
  bool setY(int y) override;
  bool setSize(int width, int height) override;
  bool setWidth(int width) override;
  bool setHeight(int height) override;
  bool setRect(int x, int y, int width, int height) override;

  bool enable() override;
  bool disable() override;
  bool showArrows();
  bool hideArrows();
  bool setReadOnly();
  bool setEditable();

  bool setMin(int64_t min);
  bool setMax(int64_t max);
  bool setCurrentValue(int64_t v);
  int64_t getCurrentValue() const;

  static UpDownPicker create(HWND hParent, int x, int y, int width, int height);
};