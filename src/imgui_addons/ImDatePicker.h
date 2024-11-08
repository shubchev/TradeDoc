#pragma once

#include <cpp-utils/common.h>
#include <cpp-utils/osal.h>

enum class ImDatePickerResult {
  Ok = 0, // returned when the picker is rendering but no selection is made
  Cancel, // returned when the picker is not rendered and no selection is made
  NewSel, // returned when the picker is not rendered and selection is made
};

class ImDatePicker {
protected:
  CalendarTime date;
  String format = "%d/%m/%Y";
  bool openPopup = false;
  bool mondayFirst = true;
  bool openFlag = false;

public:
  ImDatePicker();

  // Default format is "%d/%m/%Y"
  bool setFormat(const String &format);
  void setFirstDaySunday(bool yes);
  bool setCurrentDate(const CalendarTime &date);

  void open();
  void close();

  ImDatePickerResult render(const String &label, bool closeWhenMouseLeavesIt = true);

  CalendarTime getSelection() const;

  static CalendarTime getToday();
  static const char *getShortMonthName(const CalendarTime &date);
  static const char *getShortMonthName(int month);
  static const char *getLongMonthName(const CalendarTime &date);
  static const char *getLongMonthName(int month);
};