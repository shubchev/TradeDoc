#include "../imgui/ImGui.hpp"

#include "ImDatePicker.h"
#include <time.h>
#include <ctype.h>

static const char *longMonthName[] = {
  "January", "February", "March",
  "April", "May", "June",
  "July", "August", "September",
  "October", "November", "December",
};

static const char *shortMonthName[] = {
  "Jan", "Feb", "Mar",
  "Apr", "May", "Jun",
  "Jul", "Aug", "Sep",
  "Oct", "Nov", "Dec",
};


ImDatePicker::ImDatePicker() {
  date = getToday();
}

CalendarTime ImDatePicker::getToday() {
  return getCalendarTime();
}

const char *ImDatePicker::getShortMonthName(const CalendarTime &date) {
  if (date.month < 1 || date.month > 12) return "";
  return shortMonthName[date.month - 1];
}

const char *ImDatePicker::getShortMonthName(int month) {
  if (month < 1 || month > 12) return "";
  return shortMonthName[month - 1];
}

const char *ImDatePicker::getLongMonthName(const CalendarTime &date) {
  if (date.month < 1 || date.month > 12) return "";
  return longMonthName[date.month - 1];
}

const char *ImDatePicker::getLongMonthName(int month) {
  if (month < 1 || month > 12) return "";
  return longMonthName[month - 1];
}

bool ImDatePicker::setFormat(const String &fmt) {
  format = fmt;
  return true;
}

void ImDatePicker::setFirstDaySunday(bool yes) {
  mondayFirst = !yes;
}

bool ImDatePicker::setCurrentDate(const CalendarTime &d) {
  date = d;
  return true;
}

CalendarTime ImDatePicker::getSelection() const {
  return date;
}

void ImDatePicker::open() {
  openFlag = true;
}
void ImDatePicker::close() {
  openFlag = false;
}

ImDatePickerResult ImDatePicker::render(const String &label, bool closeWhenMouseLeavesIt) {
  auto &g = *GImGui;
  auto style = g.Style;
  auto window = ImGui::GetCurrentWindow();
  if (window->SkipItems) {
    return ImDatePickerResult::Cancel;
  }

  char dateTxt[32];
  SPRINTF(dateTxt, "%02d %s %04d",
              date.day, ImDatePicker::getShortMonthName(date), date.year);
  String btnName = dateTxt + label + "##dpBtn";
  String popName = label + "##dpPop";

  if (ImGui::Button(btnName.c_str()) || openFlag) {
    if (!openPopup) {
      ImGui::OpenPopup(popName.c_str());
      openPopup = true;
    }
  }

  auto finalResult = ImDatePickerResult::Ok;
  if (openPopup) {
    vec2 popSize = ImGui::GetItemRectSize();
    float calWidth = ImGui::CalcTextSize(longMonthName[8]).x +
                     4 * (ImGui::CalcTextSize("<").x + ImGui::CalcTextSize(">").x) +
                     ImGui::CalcTextSize("9999").x + 10 * style.FramePadding.x +
                     2 * style.ItemSpacing.x;
    popSize.x = max(popSize.x, calWidth);
    popSize.y = 6 * ImGui::GetTextLineHeightWithSpacing();

    ImGui::SetNextWindowPos(vec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
    ImGui::SetNextWindowSize(popSize);
    if (ImGui::BeginPopup(popName.c_str())) {

      ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Button, vec4(0));
        if (ImGui::SmallButton("<")) {

        }
        ImGui::SameLine();
        ImGui::Text(longMonthName[date.month - 1]);
        ImGui::SameLine();
        if (ImGui::SmallButton(">")) {

        }
        ImGui::SameLine();
        if (ImGui::SmallButton("<")) {

        }
        ImGui::SameLine();
        char yearStr[16];
        SPRINTF(yearStr, "%04d", 1900 + date.year);
        ImGui::Text(yearStr);
        ImGui::SameLine();
        if (ImGui::SmallButton(">")) {

        }
        ImGui::PopStyleColor();
      ImGui::EndGroup();

      ImGui::Separator();

      static char *dayName[2][7] = {
        { "S", "M", "T", "W", "T", "F", "S" },
        { "M", "T", "W", "T", "F", "S", "S" },
      };
      ImGui::BeginGroup();
      float dx = (popSize.x - 2 * style.FramePadding.x) / 7;
      float cPos = ImGui::GetCursorPosX();
      for (int i = 0; i < 7; i++) {
        ImGui::SetCursorPosX(cPos + i * dx);
        auto dn = dayName[mondayFirst];
        if (dn[i][0] == 'S') ImGui::TextColored(Color_Red, dn[i]);
        else ImGui::Text(dn[i]);
        ImGui::SameLine();
      }
      ImGui::EndGroup();


      ImGui::Separator();




      if (!openFlag) {
        ImGui::CloseCurrentPopup();
        finalResult = ImDatePickerResult::Cancel;
      }

      ImGui::EndPopup();
    } else {
      openFlag = false;
      openPopup = false;
      finalResult = ImDatePickerResult::Cancel;
    }
  }

  return finalResult;
}