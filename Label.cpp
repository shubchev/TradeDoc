#include "Label.h"

class LabelImpl : public ILabel {
public:
  LabelImpl(HWND parent, int x, int y, int width, int height)
    : ILabel(parent, x, y, width, height) {}
};

Label ILabel::create(HWND hParent, int x, int y, int width, int height) {
  return MakeHandle(LabelImpl, hParent, x, y, width, height);
}

ILabel::ILabel(HWND parent, int x, int y, int width, int height) {
  if (!createWindow(parent, TEXT("Static"),
                    WS_CHILD | WS_VISIBLE, x, y, width, height)) {
    throw std::runtime_error("");
  }
}

bool ILabel::setText(const wchar_t *fmt, ...) {
  std::wstring txt;
  va_list vl;
  va_start(vl, fmt);
  bool ret = vlFmtToStr(fmt, vl, txt);
  va_end(vl);
  ret = ret && setText(txt);
  return ret;
}

bool ILabel::setText(const std::wstring &text) {
  auto r = getTextRect(hWnd, text);
  if (setSize(abs(r.right - r.left), abs(r.top - r.bottom))) {
    return IWinSubclass::setText(text);
  }
  return false;
}

int ILabel::getTextLength(HWND hWnd, const std::wstring &text) {
  auto r = getTextRect(hWnd, text);
  return abs(r.left - r.right);
}

RECT ILabel::getTextRect(HWND hWnd, const std::wstring &text) {
  RECT r = { 0, 0, 0, 0 };
  if (text.length()) {
    auto dc = GetDC(hWnd);
    int len = (int)text.length();
    if (text.back() == 0) len--;
    DrawText(dc, text.c_str(), len, &r, DT_CALCRECT);
    ReleaseDC(hWnd, dc);
  }

  return r;
}