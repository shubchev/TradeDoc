#pragma once

#include <cpp-utils/common.h>
#include <cpp-utils/osal.h>

enum class ImTextFieldFormat {
  All = 0,
  Password,
  Number,
  PositiveNumber,
  Integer,
  PositiveInteger,
};

struct ImGuiInputTextCallbackData;

class ImTextField {
protected:
  String buf;
  ImTextFieldFormat format = ImTextFieldFormat::All;
  bool focused = false;
  bool setFocus = false;

  static int FilterNumber(ImGuiInputTextCallbackData *data);
  static int FilterPositiveNumber(ImGuiInputTextCallbackData *data);
  static int FilterInteger(ImGuiInputTextCallbackData *data);
  static int FilterPositiveInteger(ImGuiInputTextCallbackData *data);

public:
  ImTextField();

  ImTextFieldFormat getFormat() const { return format; }
  bool setFormat(ImTextFieldFormat format);
  bool setText(const String &str);
  bool setText(const char *fmt, ...);
  void clearText();

  bool render(const String &label);

  void focus();
  bool hasFocus() const { return focused; }

  String getText() const;
};