#pragma once

#include <inttypes.h>
#include <cpp-utils/common.h>
#include <cpp-utils/opengl.h>
#include <cpp-utils/xml.h>
#include <cpp-utils/compress.h>
#include "imgui/ImGui.hpp"
#include "imgui_addons/ImTextField.h"
#include "imgui_addons/ImDatePicker.h"


extern uintptr_t g_hInst;
extern void ImTooltip(const Color &color, const char *fmt, ...);
extern void ImTooltip(const char *fmt, ...);
