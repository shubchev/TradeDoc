#pragma once

#include "common.h"

bool vlFmtToStr(const wchar_t *fmt, va_list vl, std::wstring &out);
std::wstring vlFmtToStr(const wchar_t *fmt, ...);

DefineHandle(IWinEventCb, WinEventCb);
DefineHandle(IWinSubclass, WinSubclass);

class IWinEventCb {
public:
  virtual ~IWinEventCb() {}
  virtual void onSetFocus(IWinSubclass *win) {}
  virtual void onKillFocus(IWinSubclass *win) {}
  virtual bool OnWinEvent(IWinSubclass *win, UINT msg, WPARAM wParam, LPARAM lParam) {
    return false;
  }
};

class IWinSubclass {
protected:
  bool createWindow(HWND parent, LPCWSTR className, DWORD style,
                    int x, int y, int width , int height);

  HWND hParent = NULL;
  HWND hWnd = NULL;
  POINT pos = { 0, 0 };
  SIZE size = { 0, 0 };


  virtual bool setText(const wchar_t *fmt, va_list vl);

  std::vector<WinEventCbRef> callbacks;

public:
  virtual ~IWinSubclass();

  HWND getHandle() const { return hWnd; }

  void addEventCb(const WinEventCb &cb);

  virtual bool setText(const std::wstring &text);
  virtual bool setText(const wchar_t *fmt, ...);
  virtual std::wstring getText() const;

  virtual bool show();
  virtual bool hide();
  virtual bool enable();
  virtual bool disable();

  virtual bool setFont(HFONT font);

  virtual bool setPos(int x, int y);
  virtual bool setX(int x);
  virtual bool setY(int y);
  virtual bool setSize(int width, int height);
  virtual bool setWidth(int width);
  virtual bool setHeight(int height);
  virtual bool setRect(int x, int y, int width, int height);
  int getX() const { return pos.x; }
  int getY() const { return pos.y; }
  int getWidth() const { return size.cx; }
  int getHeight() const { return size.cy; }

  virtual bool isPointOver(int x, int y) const;

  virtual bool OnEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return false; }
  virtual bool OnNotify(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return false; }

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg,
                                  WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
                                  DWORD_PTR dwRefData);
};