#include "TextBox.h"

class TextBoxImpl : public ITextBox {
public:
  TextBoxImpl(HWND parent, int x, int y, int width, int height)
    : ITextBox(parent, x, y, width, height) {
  }
};

TextBox ITextBox::create(HWND hParent, int x, int y, int width, int height) {
  return MakeHandle(TextBoxImpl, hParent, x, y, width, height);
}

ITextBox::ITextBox(HWND parent, int x, int y, int width, int height) {
  if (!createWindow(parent, TEXT("Edit"),
                    WS_CHILD | WS_VISIBLE, x, y, width, height)) {
    throw std::runtime_error("");
  }
}

void ITextBox::setTextEventCb(const TextBoxCb &cb) {
  textCb = cb;
}

bool ITextBox::setTextFilter(TextFilter f) {
  if (f < TextFilter::Password || f > TextFilter::All) return false;

  bool ret = true;
  if (f == TextFilter::Password && filter != f) {
    auto style = GetWindowLong(hWnd, GWL_STYLE);
    ret = SetWindowLong(hWnd, GWL_STYLE, style | ES_PASSWORD);
  } else if (f != TextFilter::Password && filter == TextFilter::Password) {
    auto style = GetWindowLong(hWnd, GWL_STYLE);
    ret = SetWindowLong(hWnd, GWL_STYLE, style & (~ES_PASSWORD));
  }

  if (ret) filter = f;
  return ret;
}

bool ITextBox::setReadOnly() {
  auto style = GetWindowLong(hWnd, GWL_STYLE);
  return SetWindowLong(hWnd, GWL_STYLE, style | EM_SETREADONLY);
}
bool ITextBox::setEditable() {
  auto style = GetWindowLong(hWnd, GWL_STYLE);
  return SetWindowLong(hWnd, GWL_STYLE, style & (~EM_SETREADONLY));
}


bool ITextBox::setText(const wchar_t *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
  std::wstring out;
  auto ret = vlFmtToStr(fmt, vl, out);
  va_end(vl);
  if (ret) setText(out);
  return ret;
}

bool isReal(const std::wstring &str, bool mustBePositive = false) {
  int dots = 0;
  for (size_t i = 0; i < str.length() && dots <= 1; i++) {
    if (!str[i]) break;
    if (isdigit(str[i])) continue;
    if (str[i] == L'.') { dots++; continue; }
    if (i == 0) {
      if (str[i] == '+') continue;
      if (!mustBePositive && str[i] == '-') continue;
    }
    return false;
  }
  return dots <= 1;
}

bool isInteger(const std::wstring &str, bool mustBePositive = false) {
  for (size_t i = 0; i < str.length(); i++) {
    if (!str[i]) break;
    if (isdigit(str[i])) continue;
    if (i == 0) {
      if (str[i] == '+') continue;
      if (!mustBePositive && str[i] == '-') continue;
    }
    return false;
  }
  return true;
}

bool ITextBox::setText(const std::wstring &txt) {
  switch (filter) {
    case TextFilter::All:
    case TextFilter::Password: {
      auto cb = textCb.lock();
      auto newText = txt;
      if (cb && !cb->onTextChange(this, newText)) {
        return false;
      }
      std::swap(text, newText);
      SetWindowText(hWnd, text.c_str());
      return true;
    }
    case TextFilter::Number:
    case TextFilter::PositiveNumber:
    case TextFilter::IntegerNumber:
    case TextFilter::PositiveInteger: {
      auto cb = textCb.lock();
      auto newText = txt;
      if (cb && !cb->onTextChange(this, newText)) {
        return false;
      }
      if (filter == TextFilter::Number && !isReal(newText)) return false;
      if (filter == TextFilter::PositiveNumber && !isReal(newText, true)) return false;
      if (filter == TextFilter::IntegerNumber && !isInteger(newText)) return false;
      if (filter == TextFilter::PositiveInteger && !isInteger(newText, true)) return false;
      std::swap(text, newText);
      SetWindowText(hWnd, text.c_str());
      return true;
    }
    default: return false;
  }
}


std::wstring ITextBox::getText() const {
  if (filter == TextFilter::All || filter == TextFilter::Password) {
    text.resize(GetWindowTextLength(hWnd));
    GetWindowText(hWnd, &text[0], (int)text.length());
  }
  return text;
}

bool ITextBox::OnEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (uMsg == WM_KEYDOWN) {
    if (wParam == VK_DELETE) {
      DWORD selBeg = 0, selEnd = 0;
      SendMessage(hWnd, EM_GETSEL, (WPARAM)&selBeg, (LPARAM)&selEnd);
      auto t = getText();
      if (selBeg != selEnd) t.erase(t.begin() + selBeg, t.begin() + selEnd);
      else if (selBeg < t.length()) t.erase(t.begin() + selBeg, t.begin() + selBeg + 1);
      if (setText(t)) {
        SendMessage(hWnd, EM_SETSEL, (WPARAM)selBeg, (LPARAM)selBeg);
      }
      return true;
    }
  }
  if (uMsg == WM_CHAR) {
    auto shift = (uint16_t)GetAsyncKeyState(VK_SHIFT);
    if (wParam && wParam <= 255 && isalpha((int)wParam)) {
      wParam = (WPARAM)toupper((int)wParam);
    }
    DWORD selBeg = 0, selEnd = 0;
    SendMessage(hWnd, EM_GETSEL, (WPARAM)&selBeg, (LPARAM)&selEnd);

    bool reject = true;
    switch (filter) {
      case TextFilter::All:
      case TextFilter::Password:
        if (wParam == VK_ESCAPE) return true;
        break;
      case TextFilter::Number:
      case TextFilter::PositiveNumber:
      case TextFilter::IntegerNumber:
      case TextFilter::PositiveInteger:
        if ((wParam >= '0' && wParam <= '9') ||
            (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9) ||
             wParam == '-' || wParam == '.') {
          auto t = getText();
          if (selBeg != selEnd) {
            t.erase(t.begin() + selBeg, t.begin() + selEnd);
          }
          t.insert(t.begin() + selBeg, (wchar_t)wParam);
          if (setText(t)) {
            selBeg++;
            SendMessage(hWnd, EM_SETSEL, (WPARAM)selBeg, (LPARAM)selBeg);
          }
        }
        if (wParam == VK_BACK) {
          auto t = getText();
          if (selBeg != selEnd) t.erase(t.begin() + selBeg, t.begin() + selEnd);
          else if (selBeg > 0) {
            selBeg--;
            t.erase(t.begin() + selBeg, t.begin() + selBeg + 1);
          }
          if (setText(t)) {
            SendMessage(hWnd, EM_SETSEL, (WPARAM)selBeg, (LPARAM)selBeg);
          }
        }
        return true;
      default: return true;
    }
  }

  return false;
}
