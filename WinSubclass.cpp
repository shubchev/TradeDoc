#include "WinSubclass.h"
#include <CommCtrl.h>



bool vlFmtToStr(const wchar_t *fmt, va_list vl, std::wstring &txt) {
  auto len = (size_t)(_vscwprintf(fmt, vl) + 1); // not counting terminating '\0'
  txt.resize(len, 0);
  bool ret = false;
  if (txt.size() == len) {
    vswprintf_s(&txt[0], len, fmt, vl);
    ret = true;
  }
  return ret;
}

std::wstring vlFmtToStr(const wchar_t *fmt, ...) {
  std::wstring out;
  va_list vl;
  va_start(vl, fmt);
  vlFmtToStr(fmt, vl, out);
  va_end(vl);
  return out;
}




IWinSubclass::~IWinSubclass() {
  if (hWnd) {
    RemoveWindowSubclass(hWnd, IWinSubclass::WndProc, (UINT_PTR)this);
    DestroyWindow(hWnd);
  }
}

bool IWinSubclass::createWindow(HWND parent, LPCWSTR className, DWORD style,
                               int x, int y, int width, int height) {
  auto wnd = CreateWindowEx(0, className, TEXT(""), style,
                            x, y, width, height,
                            parent, NULL, g_hInst, NULL);
  if (!wnd) {
    return false;
  }
  if (!SetWindowSubclass(wnd, IWinSubclass::WndProc, (UINT_PTR)this, 0)) {
    DestroyWindow(wnd);
    return false;
  }
  hWnd = wnd;
  hParent = parent;
  pos.x = x; pos.y = y;
  size.cx = width; size.cy = height;

  return true;
}

void IWinSubclass::addEventCb(const WinEventCb &cb) {
  callbacks.push_back(cb);
}

LRESULT CALLBACK IWinSubclass::WndProc(HWND hWnd, UINT msg,
                                      WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
                                      DWORD_PTR dwRefData) {
  auto pThis = (IWinSubclass *)uIdSubclass;
  auto processed = pThis->OnEvent(hWnd, msg, wParam, lParam);
  for (auto it = pThis->callbacks.begin(); it != pThis->callbacks.end();) {
    if (auto cb = (*it).lock()) {
      if (!processed) processed = cb->OnWinEvent(pThis, msg, wParam, lParam);
      it++;
    } else it = pThis->callbacks.erase(it);
  }
  if (processed) return 0;
  return DefSubclassProc(hWnd, msg, wParam, lParam);
}


bool IWinSubclass::setText(const wchar_t *fmt, va_list vl) {
  std::wstring txt;
  bool ret = vlFmtToStr(fmt, vl, txt);
  ret = ret && IWinSubclass::setText(txt);
  return ret;
}

bool IWinSubclass::setText(const wchar_t *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
  bool ret = IWinSubclass::setText(fmt, vl);
  va_end(vl);
  return ret;
}

bool IWinSubclass::setText(const std::wstring &text) {
  return SetWindowText(hWnd, text.c_str());
}

std::wstring IWinSubclass::getText() const {
  auto len = GetWindowTextLength(hWnd) + 1;
  std::wstring txt(len, 0);
  GetWindowText(hWnd, &txt[0], len);
  return txt;
}

bool IWinSubclass::show() {
  return SetWindowPos(hWnd, NULL, pos.x, pos.y, size.cx, size.cy, 0);
}
bool IWinSubclass::hide() {
  return SetWindowPos(hWnd, NULL, 100000, 100000, size.cx, size.cy, 0);
}
bool IWinSubclass::enable() {
  auto style = GetWindowLong(hWnd, GWL_STYLE);
  return SetWindowLong(hWnd, GWL_STYLE, style & (~WS_DISABLED));
}
bool IWinSubclass::disable() {
  auto style = GetWindowLong(hWnd, GWL_STYLE);
  return SetWindowLong(hWnd, GWL_STYLE, style | WS_DISABLED);
}

bool IWinSubclass::setFont(HFONT font) {
  return SendMessage(hWnd, WM_SETFONT, (WPARAM)font, TRUE);
}

bool IWinSubclass::setPos(int x, int y) {
  auto ret = SetWindowPos(hWnd, NULL, x, y, size.cx, size.cy, 0);
  if (ret) { pos.x = x; pos.y = y; }
  return ret;
}
bool IWinSubclass::setX(int x) {
  auto ret = SetWindowPos(hWnd, NULL, x, pos.y, size.cx, size.cy, 0);
  if (ret) pos.x = x;
  return ret;
}
bool IWinSubclass::setY(int y) {
  auto ret = SetWindowPos(hWnd, NULL, pos.x, y, size.cx, size.cy, 0);
  if (ret) pos.y = y;
  return ret;
}
bool IWinSubclass::setSize(int width, int height) {
  auto ret = SetWindowPos(hWnd, NULL, pos.x, pos.y, width, height, 0);
  if (ret) { size.cx = width; size.cy = height; }
  return ret;
}
bool IWinSubclass::setWidth(int width) {
  auto ret = SetWindowPos(hWnd, NULL, pos.x, pos.y, width, size.cy, 0);
  if (ret) size.cx = width;
  return ret;
}
bool IWinSubclass::setHeight(int height) {
  auto ret = SetWindowPos(hWnd, NULL, pos.x, pos.y, size.cx, height, 0);
  if (ret) size.cy = height;
  return ret;
}
bool IWinSubclass::setRect(int x, int y, int width, int height) {
  auto ret = SetWindowPos(hWnd, NULL, x, y, width, height, 0);
  if (ret) { pos.x = x; pos.y = y; size.cx = width; size.cy = height; }
  return ret;
}

bool IWinSubclass::isPointOver(int x, int y) const {
  return x >= pos.x           && y >= pos.y &&
         x <= pos.x + size.cx && y <= pos.y + size.cy;
}
