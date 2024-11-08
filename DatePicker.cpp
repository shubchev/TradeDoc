#include "DatePicker.h"
#include <CommCtrl.h>

class DatePickerImpl : public IDatePicker {
public:
  DatePickerImpl(HWND parent, int x, int y, int width, int height)
    : IDatePicker(parent, x, y, width, height) {
  }
};

DatePicker IDatePicker::create(HWND hParent, int x, int y, int width, int height) {
  return MakeHandle(DatePickerImpl, hParent, x, y, width, height);
}


IDatePicker::IDatePicker(HWND parent, int x, int y, int width, int height) {
  if (!createWindow(parent, DATETIMEPICK_CLASS,
                    WS_BORDER | WS_CHILD | WS_VISIBLE,
                    x, y, width, height)) {
    throw std::runtime_error("");
  }

  //MonthCal_SetFirstDayOfWeek(hWnd, 0); // first day Monday
  MonthCal_SetMaxSelCount(hWnd, 1); // only one day selection
  SYSTEMTIME range[2];
  DateTime_GetSystemtime(hWnd, &range[1]);
  DateTime_SetRange(hWnd, GDTR_MAX, range);
}

SYSTEMTIME IDatePicker::getValue() const {
  SYSTEMTIME t;
  DateTime_GetSystemtime(hWnd, &t);
  return t;
}
bool IDatePicker::setValue(const SYSTEMTIME &time) {
  return DateTime_SetSystemtime(hWnd, GDT_VALID , &time);
}

bool IDatePicker::OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

  return false;
}
