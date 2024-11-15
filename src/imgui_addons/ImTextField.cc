#include "../imgui/ImGui.hpp"

#include "ImTextField.h"
#include <time.h>
#include <ctype.h>

ImTextField::ImTextField() {
  buf.resize(1025);
  MemSet(buf.data(), 0, buf.size());
}

bool ImTextField::setFormat(ImTextFieldFormat fmt) {
  format = fmt;
  return true;
}

bool ImTextField::setText(const String &t) {
  MemCopy(buf.data(), t.data(), min(buf.size(), t.size()));
  buf.back() = 0;
  return true;
}

bool ImTextField::setText(const char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
  vsnprintf(buf.data(), buf.size(), fmt, vl);
  va_end(vl);
  buf.back() = 0;
  return true;
}

void ImTextField::clearText() {
  MemSet(buf.data(), 0, buf.size());
}

String ImTextField::getText() const {
  return String(buf.c_str());
}

void ImTextField::focus() {
  setFocus = true;
}

bool isReal(const String &str, bool positiveOnly) {
  auto ucStr = toUString(str);
  int dots = 0;
  for (size_t i = 0; i < ucStr.length() && dots <= 1; i++) {
    auto c = ucStr[i];
    if (c == 0) break;
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

bool isInteger(const String &str, bool positiveOnly) {
  auto ucStr = toUString(str);
  for (size_t i = 0; i < ucStr.length(); i++) {
    auto c = ucStr[i];
    if (c == 0) break;
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
  if (setFocus) {
    ImGui::SetKeyboardFocusHere(0);
    focused = true;
    setFocus = false;
  }

  switch (format) {
    case ImTextFieldFormat::All:
      ImGui::InputText(label.c_str(), buf.data(), buf.size());
      break;
    case ImTextFieldFormat::Password:
      ImGui::InputText(label.c_str(), buf.data(), buf.size(),
                       ImGuiInputTextFlags_Password);
      break;
    case ImTextFieldFormat::Number:
      ImGui::InputText(label.c_str(), buf.data(), buf.size(),
                       ImGuiInputTextFlags_CallbackCharFilter |
                       ImGuiInputTextFlags_CallbackAlways,
                       FilterNumber, this);
      break;
    case ImTextFieldFormat::PositiveNumber:
      ImGui::InputText(label.c_str(), buf.data(), buf.size(),
                       ImGuiInputTextFlags_CallbackCharFilter |
                       ImGuiInputTextFlags_CallbackAlways,
                       FilterPositiveNumber, this);
      break;
    case ImTextFieldFormat::Integer:
      ImGui::InputText(label.c_str(), buf.data(), buf.size(),
                       ImGuiInputTextFlags_CallbackCharFilter |
                       ImGuiInputTextFlags_CallbackAlways,
                       FilterInteger, this);
      break;
    case ImTextFieldFormat::PositiveInteger:
      ImGui::InputText(label.c_str(), buf.data(), buf.size(),
                       ImGuiInputTextFlags_CallbackCharFilter |
                       ImGuiInputTextFlags_CallbackAlways,
                       FilterPositiveInteger, this);
      break;
    default: return false;
  }

  return ImGui::IsItemDeactivatedAfterEdit();
}