#include "../imgui/ImGui.hpp"

#include "ImDatePicker.h"
#include <time.h>
#include <ctype.h>

static const char *longMonthName[] = {
  " January ",
  "February ",
  "  March  ",
  "  April  ",
  "   May   ",
  "  June   ",
  "  July   ",
  " August  ",
  "September",
  " October ",
  "November ",
  "December ",
};

static const char *shortMonthName[] = {
  "Jan", "Feb", "Mar",
  "Apr", "May", "Jun",
  "Jul", "Aug", "Sep",
  "Oct", "Nov", "Dec",
};


ImDatePicker::ImDatePicker() {
  date = CalendarDate::today();
}

const char *ImDatePicker::getShortMonthName(const CalendarDate &date) {
  if (!date.isValid()) return "";
  return shortMonthName[date.month() - 1];
}

const char *ImDatePicker::getShortMonthName(int month) {
  if (month < 1 || month > 12) return "";
  return shortMonthName[month - 1];
}

const char *ImDatePicker::getLongMonthName(const CalendarDate &date) {
  if (!date.isValid()) return "";
  return longMonthName[date.month() - 1];
}

const char *ImDatePicker::getLongMonthName(int month) {
  if (month < 1 || month > 12) return "";
  return longMonthName[month - 1];
}

void ImDatePicker::setFormat(DateFormat fmt) {
  format = fmt;
}

void ImDatePicker::setFirstDaySunday(bool yes) {
  mondayFirst = !yes;
}

void ImDatePicker::setButtonTextColor(const Color &color) {
  buttonTextColor = color;
}

bool ImDatePicker::setCurrentDate(const CalendarDate &d) {
  date = d;
  return date.isValid();
}

CalendarDate ImDatePicker::getSelection() const {
  return date;
}

void ImDatePicker::open() {
  openFlag = true;
  viewDate = date;
}
void ImDatePicker::close() {
  closeFlag = true;
}

ImDatePickerResult ImDatePicker::render(const String &label, bool closeWhenMouseLeavesIt) {
  auto &g = *GImGui;
  auto style = g.Style;
  auto window = ImGui::GetCurrentWindow();
  if (window->SkipItems) {
    return ImDatePickerResult::Cancel;
  }

  if (dateText.empty()) {
    dateText = date.str(format);
    btnName = dateText + "##" + label + "##dpBtn";
  }
  if (popName.empty()) popName = "##" + label + "##dpPop";


  ImGui::PushStyleColor(ImGuiCol_Text, buttonTextColor);
  if (ImGui::Button(&btnName[0])) {
    open();
  }
  ImGui::PopStyleColor();
  btnRect = Rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

  auto finalResult = ImDatePickerResult::Ok;

  if (openFlag) {
    ImGui::OpenPopup(&popName[0]);
    openFlag = false;
    opened = true;
  }

  float calWidth = 0.0f;
  if (opened) {
    calWidth = ImGui::CalcTextSize(longMonthName[8]).x +
               4 * (ImGui::CalcTextSize("<").x + ImGui::CalcTextSize(">").x) +
               ImGui::CalcTextSize("9999").x + 10 * style.FramePadding.x +
               2 * style.ItemSpacing.x;

    popSize = btnRect.size();
    popSize.x = max(popSize.x, calWidth);
    popSize.y = 9 * ImGui::GetTextLineHeightWithSpacing();

    ImGui::SetNextWindowPos(vec2(btnRect.pos().x, btnRect.posMax().y));
    ImGui::SetNextWindowSize(popSize);
  }
  if (ImGui::BeginPopup(&popName[0])) {
    ImGui::BeginGroup();
    {
      ImGui::PushStyleColor(ImGuiCol_Button, vec4(0));
      if (ImGui::SmallButton("<##month")) {
        viewDate >>= 1;
      }
      ImGui::SameLine();
      ImGui::Text(longMonthName[viewDate.month() - 1]);
      ImGui::SameLine();
      if (ImGui::SmallButton(">##month")) {
        viewDate <<= 1;
      }
      ImGui::SameLine();
      if (ImGui::SmallButton("<##year")) {
        viewDate.decYear();
      }
      ImGui::SameLine();
      char yearStr[16];
      SPRINTF(yearStr, "%04d", viewDate.year());
      ImGui::Text(yearStr);
      ImGui::SameLine();
      if (ImGui::SmallButton(">##year")) {
        viewDate.incYear();
      }
      ImGui::PopStyleColor();
    }
    ImGui::EndGroup();

    ImGui::Separator();

    static char *dayName[2][7] = {
      { "S", "M", "T", "W", "T", "F", "S" },
      { "M", "T", "W", "T", "F", "S", "S" },
    };
    ImGui::BeginGroup();
    {
      float dx = (popSize.x - 2 * style.FramePadding.x) / 7;
      float cPos = ImGui::GetCursorPosX();
      cPos += ImGui::CalcTextSize("M").x;
      for (int i = 0; i < 7; i++) {
        ImGui::SetCursorPosX(cPos + i * dx);
        auto dn = dayName[mondayFirst];
        if (dn[i][0] == 'S') ImGui::TextColored(Color_Red, dn[i]);
        else ImGui::Text(dn[i]);
        ImGui::SameLine();
      }
    }
    ImGui::EndGroup();


    ImGui::Separator();

    ImGui::BeginGroup();
    {
      auto fmd = CalendarDate(1, viewDate.month(), viewDate.year());
      int i = fmd.dayOfWeek() - mondayFirst;
      int j = 0;
      int nDays = viewDate.daysInTheMonth();
      float dx = (popSize.x - 2 * style.FramePadding.x) / 7;
      vec2 cPos = ImGui::GetCursorPos();

      ImGui::PushStyleColor(ImGuiCol_Button, vec4(0));

      viewDate >>= 1;
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
      for (int k = 0, n = viewDate.daysInTheMonth() - i + 1; k < i; k++, n++) {
        char txt[8];
        SPRINTF(txt, "%d##%d", n, viewDate.month());

        ImGui::SetCursorPos(cPos + vec2(k * dx, 0));
        if (ImGui::SmallButton(txt)) {
          date = CalendarDate(n, viewDate.month(), viewDate.year());
          dateText.clear();
          finalResult = ImDatePickerResult::NewSel;
          closeFlag = true;
        }
      }
      ImGui::PopStyleVar();
      viewDate <<= 1;

      for (int n = 1; n <= nDays; n++) {
        char txt[8];
        if (n < 10) SPRINTF(txt, " %d##%d", n, viewDate.month());
        else SPRINTF(txt, "%d##%d", n, viewDate.month());

        ImGui::SetCursorPos(cPos + vec2(i * dx, 0));
        bool btnPress = false;
        if (date.day() == n &&
            date.month() == viewDate.month() &&
            date.year() == viewDate.year()) {
          ImGui::PopStyleColor();
          btnPress = ImGui::SmallButton(txt);
          ImGui::PushStyleColor(ImGuiCol_Button, vec4(0));
        } else {
          btnPress = ImGui::SmallButton(txt);
        }

        if (btnPress) {
          date = CalendarDate(n, viewDate.month(), viewDate.year());
          dateText.clear();
          finalResult = ImDatePickerResult::NewSel;
          closeFlag = true;
        }

        i++;
        if (i == 7) {
          i = 0;
          j++;
          cPos.y += ImGui::GetTextLineHeightWithSpacing();
        }
      }


      viewDate <<= 1;
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
      for (int n = 1; i < 7; i++, n++) {
        char txt[8];
        SPRINTF(txt, " %d##%d", n, viewDate.month());

        ImGui::SetCursorPos(cPos + vec2(i * dx, 0));
        if (ImGui::SmallButton(txt)) {
          date = CalendarDate(n, viewDate.month(), viewDate.year());
          dateText.clear();
          finalResult = ImDatePickerResult::NewSel;
          closeFlag = true;
        }
      }
      ImGui::PopStyleVar();
      viewDate >>= 1;

      ImGui::PopStyleColor();
    }
    ImGui::EndGroup();


    opened = true;
    if (closeFlag) {
      ImGui::CloseCurrentPopup();
      if (finalResult == ImDatePickerResult::Ok)
        finalResult = ImDatePickerResult::Cancel;
      closeFlag = false;
    }
    ImGui::EndPopup();
  } else {
    if (opened) {
      finalResult = ImDatePickerResult::Cancel;
    }
    opened = false;
  }

  return finalResult;
}