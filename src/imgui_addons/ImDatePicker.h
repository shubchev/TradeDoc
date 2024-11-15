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
  Color buttonTextColor = Color_White;
  CalendarDate date;
  CalendarDate viewDate;
  DateFormat format = DateFormat::DMY;
  bool mondayFirst = true;
  bool opened = false;
  bool openFlag = false;
  bool closeFlag = false;

  String dateText;
  String btnName;
  String popName;
  Rect btnRect;
  vec2 popSize;

public:
  ImDatePicker();

  // Default format is "%d/%m/%Y"
  void setFormat(DateFormat format);
  void setFirstDaySunday(bool yes);
  void setButtonTextColor(const Color &color);
  bool setCurrentDate(const CalendarDate &date);

  void open();
  void close();

  ImDatePickerResult render(const String &label, bool closeWhenMouseLeavesIt = true);

  CalendarDate getSelection() const;

  static const char *getShortMonthName(const CalendarDate &date);
  static const char *getShortMonthName(int month);
  static const char *getLongMonthName(const CalendarDate &date);
  static const char *getLongMonthName(int month);
};