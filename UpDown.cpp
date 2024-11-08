#include "UpDown.h"
#include <CommCtrl.h>
#include <limits>
#include <inttypes.h>

#undef min
#undef max

class UpDownPickerImpl : public IUpDownPicker {
public:
  UpDownPickerImpl(HWND parent, int x, int y, int width, int height)
    : IUpDownPicker(parent, x, y, width, height) {
  }
};

UpDownPicker IUpDownPicker::create(HWND hParent, int x, int y, int width, int height) {
  auto udp = MakeHandle(UpDownPickerImpl, hParent, x, y, width, height);
  if (udp) {
    udp->text->addEventCb(udp);
    udp->text->setTextEventCb(udp);
  }
  return udp;
}


IUpDownPicker::IUpDownPicker(HWND parent, int x, int y, int width, int height) {
  udSize = height;
  text = ITextBox::create(parent, x, y, width - udSize, height);
  if (!text) {
    throw std::runtime_error("");
  }
  text->setTextFilter(TextFilter::PositiveInteger);

  if (!createWindow(parent, UPDOWN_CLASS,
                    WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS | UDS_ALIGNLEFT | UDS_SETBUDDYINT,
                    x + width - udSize, y, udSize, height)) {
    throw std::runtime_error("");
  }
}

bool IUpDownPicker::setText(const std::wstring &txt) {
  auto oldV = fmt;
  fmt = txt;
  if (!setCurrentValue(getCurrentValue())) {
    fmt = oldV;
    return false;
  }
  return true;
}
bool IUpDownPicker::setText(const wchar_t *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
  std::wstring txt;
  auto ret = vlFmtToStr(fmt, vl, txt);
  va_end(vl);
  return ret && setText(txt);
}

std::wstring IUpDownPicker::getText() const {
  return text->getText();
}
bool IUpDownPicker::show() {
  if (!text->show()) return false;
  if (!IWinSubclass::show()) { text->hide(); return false; }
  return true;
}
bool IUpDownPicker::hide() {
  if (!text->hide()) return false;
  if (!IWinSubclass::hide()) { text->show(); return false; }
  return true;
}
bool IUpDownPicker::setPos(int x, int y) {
  return text->setPos(x, y) && IWinSubclass::setPos(x + size.cx - udSize, y);
}
bool IUpDownPicker::setX(int x) {
  return text->setX(x) && IWinSubclass::setX(x + size.cx - udSize);
}
bool IUpDownPicker::setY(int y) {
  bool ret = text->setY(y);
  ret = ret && IWinSubclass::setY(y);
  return ret;
}
bool IUpDownPicker::setSize(int width, int height) {
  bool ret = text->setSize(size.cx - udSize, height);
  ret = ret && IWinSubclass::setRect(pos.x + size.cx - udSize, pos.y, udSize, height);
  return ret;
}
bool IUpDownPicker::setWidth(int width) {
  bool ret = text->setWidth(size.cx - udSize);
  ret = ret && IWinSubclass::setRect(pos.x + size.cx - udSize, pos.y, udSize, size.cy);
  return ret;
}
bool IUpDownPicker::setHeight(int height) {
  return text->setHeight(height) && IWinSubclass::setHeight(height);
}
bool IUpDownPicker::setRect(int x, int y, int width, int height) {
  bool ret = text->setRect(x, y, width - udSize, height);
  ret = ret && IWinSubclass::setRect(x + width - udSize, y, udSize, height);
  return ret;
}

bool IUpDownPicker::showArrows() {
  if (!arrows) {
    arrows = IWinSubclass::show();
    if (arrows) {
      text->setWidth(text->getWidth() - udSize);
    }
  }
  return arrows;
}
bool IUpDownPicker::hideArrows() {
  if (arrows) {
    arrows = !IWinSubclass::hide();
    if (!arrows) {
      text->setWidth(text->getWidth() + udSize);
    }
  }
  return !arrows;
}

bool IUpDownPicker::enable() {
  if (!text->enable()) return false;
  if (!IWinSubclass::enable()) { text->disable(); return false; }
  return true;
}
bool IUpDownPicker::disable() {
  if (!text->disable()) return false;
  if (!IWinSubclass::disable()) { text->enable(); return false; }
  return true;
}

bool IUpDownPicker::setReadOnly() { return text->setReadOnly(); }
bool IUpDownPicker::setEditable() { return text->setEditable(); }


#define min(a, b) ((a) < (b)) ? (a) : (b)
#define max(a, b) ((a) > (b)) ? (a) : (b)
#define clamp(v, a, b) max(a, min(v, b))

bool IUpDownPicker::setMin(int64_t min) {
  auto oldV = minV;
  minV = min;
  if (!setCurrentValue(curV)) {
    minV = oldV;
    return false;
  }
  return true;
}

bool IUpDownPicker::setMax(int64_t max) {
  auto oldV = maxV;
  maxV = max;
  if (!setCurrentValue(curV)) {
    maxV = oldV;
    return false;
  }
  return true;
}

bool IUpDownPicker::setCurrentValue(int64_t v) {
  curV = clamp(v, minV, maxV);
  return text->setText(fmt.c_str(), curV);
}

int64_t IUpDownPicker::getCurrentValue() const {
  return curV;
}


bool IUpDownPicker::onTextChange(ITextBox *tb, std::wstring &newText) {
  int64_t v = 0;
  if (swscanf_s(newText.c_str(), TEXT("%" SCNd64), &v) == 1) {
    curV = clamp(v, minV, maxV);
    newText = vlFmtToStr(fmt.c_str(), curV);
    return true;
  }
  return false;
}

bool IUpDownPicker::OnWinEvent(IWinSubclass *win, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_MOUSEWHEEL) {
    auto delta = (short)HIWORD(wParam);
    setCurrentValue(getCurrentValue() + delta / WHEEL_DELTA);
    return true;
  }

  bool processed = false;
  for (auto it = callbacks.begin(); it != callbacks.end();) {
    if (auto cb = (*it).lock()) {
      if (!processed) processed = cb->OnWinEvent(this, msg, wParam, lParam);
      it++;
    } else it = callbacks.erase(it);
  }

  return processed;
}

bool IUpDownPicker::OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (uMsg == 78) {
    auto pData = (LPNMUPDOWN)lParam;
    setCurrentValue(getCurrentValue() - pData->iDelta);
    return true;
  }
  return false;
}