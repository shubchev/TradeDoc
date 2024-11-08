#include "../imgui/ImGui.hpp"

#include "ImTextField.h"
#include <time.h>
#include <ctype.h>

ImTextField::ImTextField() {
  MemSet(buf, 0, sizeof(buf));
}

bool ImTextField::setFormat(ImTextFieldFormat fmt) {
  format = fmt;
  return true;
}

bool ImTextField::setText(const String &t) {
  strncpy_s(buf, t.c_str(), sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = 0;
  return true;
}

bool ImTextField::setText(const char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
  vsnprintf_s(buf, sizeof(buf), fmt, vl);
  va_end(vl);
  buf[sizeof(buf) - 1] = 0;
  return true;
}

String ImTextField::getText() const {
  return String(buf);
}

void ImTextField::focus() {
  setFocus = true;
}

bool isReal(const char *str, bool positiveOnly) {
  auto ucStr = toUString(str);
  int dots = 0;
  for (size_t i = 0; i < ucStr.length() && dots <= 1; i++) {
    auto c = ucStr[i];
    if (c < 256 && isdigit(c)) continue;
    if (c == '.') { dots++; continue; }
    if (i == 0) {
      if (c == '+') continue;
      if (!positiveOnly && c == '-') continue;
    }
    return false;
  }
  return dots <= 1;
}

bool isInteger(const char *str, bool positiveOnly) {
  auto ucStr = toUString(str);
  for (size_t i = 0; i < ucStr.length(); i++) {
    auto c = ucStr[i];
    if (c < 256 && isdigit(c)) continue;
    if (i == 0) {
      if (c == '+') continue;
      if (!positiveOnly && c == '-') continue;
    }
    return false;
  }
  return true;
}

int ImTextField::FilterNumber(ImGuiInputTextCallbackData *data) {
  if (data->EventChar == 0) return 1;
  if (data->EventChar >= 256 || !strchr("1234567890-.", (char)data->EventChar)) return 1;

  auto tf = (ImTextField *)data->UserData;
  if (!isReal(tf->buf, false)) return 1;
  return 0;
}

int ImTextField::FilterPositiveNumber(ImGuiInputTextCallbackData *data) {
  if (data->EventChar == 0) return 1;
  if (data->EventChar >= 256 || !strchr("1234567890.", (char)data->EventChar)) return 1;

  auto tf = (ImTextField *)data->UserData;
  if (!isReal(tf->buf, true)) return 1;
  return 0;
}

int ImTextField::FilterInteger(ImGuiInputTextCallbackData *data) {
  if (data->EventChar == 0) return 1;
  if (data->EventChar >= 256 || !strchr("1234567890-", (char)data->EventChar)) return 1;

  auto tf = (ImTextField *)data->UserData;
  if (!isInteger(tf->buf, false)) return 1;
  return 0;
}

int ImTextField::FilterPositiveInteger(ImGuiInputTextCallbackData *data) {
  if (data->EventChar == 0) return 1;
  if (data->EventChar >= 256 || !strchr("1234567890", (char)data->EventChar)) return 1;

  auto tf = (ImTextField *)data->UserData;
  if (!isInteger(tf->buf, true)) return 1;
  return 0;
}

bool ImTextField::render(const String &label) {
  bool ret = true;

  if (setFocus) {
    ImGui::SetKeyboardFocusHere(0);
    focused = true;
    setFocus = false;
  }

  switch (format) {
    case ImTextFieldFormat::All:
      ret = ImGui::InputText(label.c_str(), buf, sizeof(buf),
                       ImGuiInputTextFlags_EnterReturnsTrue);
      break;
    case ImTextFieldFormat::Password:
      ret = ImGui::InputText(label.c_str(), buf, sizeof(buf),
                       ImGuiInputTextFlags_EnterReturnsTrue |
                       ImGuiInputTextFlags_Password);
      break;
    case ImTextFieldFormat::Number:
      ret = ImGui::InputText(label.c_str(), buf, sizeof(buf),
                       ImGuiInputTextFlags_CallbackCharFilter |
                       ImGuiInputTextFlags_CallbackAlways |
                       ImGuiInputTextFlags_EnterReturnsTrue,
                       FilterNumber, this);
      break;
    case ImTextFieldFormat::PositiveNumber:
      ret = ImGui::InputText(label.c_str(), buf, sizeof(buf),
                       ImGuiInputTextFlags_CallbackCharFilter |
                       ImGuiInputTextFlags_CallbackAlways |
                       ImGuiInputTextFlags_EnterReturnsTrue,
                       FilterPositiveNumber, this);
      break;
    case ImTextFieldFormat::Integer:
      ret = ImGui::InputText(label.c_str(), buf, sizeof(buf),
                       ImGuiInputTextFlags_CallbackCharFilter |
                       ImGuiInputTextFlags_CallbackAlways |
                       ImGuiInputTextFlags_EnterReturnsTrue,
                       FilterInteger, this);
      break;
    case ImTextFieldFormat::PositiveInteger:
      ret = ImGui::InputText(label.c_str(), buf, sizeof(buf),
                       ImGuiInputTextFlags_CallbackCharFilter |
                       ImGuiInputTextFlags_CallbackAlways |
                       ImGuiInputTextFlags_EnterReturnsTrue,
                       FilterPositiveInteger, this);
      break;
    default: return false;
  }

  vec2 pos = ImGui::GetItemRectMin();
  vec2 size = ImGui::GetItemRectSize();

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    auto &io = ImGui::GetIO();
    vec2 mp = io.MousePos;
    if (!(all(greaterThanEqual(mp, pos)) && all(lessThanEqual(mp, pos + size)))) {
      focused = false;
      setText(toString(ret));
    }
  }

  return ret;
}