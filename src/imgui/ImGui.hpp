#pragma once

#include <cpp-utils/common.h>

#define IM_VEC2_CLASS_EXTRA                                                     \
  constexpr ImVec2(const vec2 &v)    : x(v.x), y(v.y) { }                       \
  ImVec2 & operator = (const vec2 &v) { x = v.x; y = v.y; return *this; }       \
  operator vec2() { return vec2(x, y); }                                        \
  ImVec2 operator +(const ImVec2 &v) { return ImVec2(x + v.x, y + v.y); }       \
  ImVec2 operator -(const ImVec2 &v) { return ImVec2(x - v.x, y - v.y); }       \
  ImVec2 operator *(const ImVec2 &v) { return ImVec2(x * v.x, y * v.y); }       \
  ImVec2 operator /(const ImVec2 &v) { return ImVec2(x / v.x, y / v.y); }       \
  ImVec2 &operator +=(const ImVec2 &v) { x += v.x, y += v.y; return *this; }    \
  ImVec2 &operator -=(const ImVec2 &v) { x -= v.x, y -= v.y; return *this; }    \
  ImVec2 &operator *=(const ImVec2 &v) { x *= v.x, y *= v.y; return *this; }    \
  ImVec2 &operator /=(const ImVec2 &v) { x /= v.x, y /= v.y; return *this; }

#define IM_VEC4_CLASS_EXTRA                                                                      \
  constexpr ImVec4(const vec4 &v)    : x(v.x), y(v.y), z(v.z), w(v.w) { }                        \
  ImVec4 & operator = (const vec4 &v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }      \
  operator vec4() { return vec4(x, y, z, w); }                                                   \
  ImVec4 operator +(const vec4 &v) { return ImVec4(x + v.x, y + v.y, z + v.z, w + v.w); }        \
  ImVec4 operator -(const vec4 &v) { return ImVec4(x - v.x, y - v.y, z - v.z, w - v.w); }        \
  ImVec4 operator *(const vec4 &v) { return ImVec4(x * v.x, y * v.y, z * v.z, w * v.w); }        \
  ImVec4 operator /(const vec4 &v) { return ImVec4(x / v.x, y / v.y, z / v.z, w / v.w); }        \
  ImVec4 &operator +=(const ImVec4 &v) { x += v.x, y += v.y, z += v.z, w += v.w; return *this; } \
  ImVec4 &operator -=(const ImVec4 &v) { x -= v.x, y -= v.y, z -= v.z, w -= v.w; return *this; } \
  ImVec4 &operator *=(const ImVec4 &v) { x *= v.x, y *= v.y, z *= v.z, w *= v.w; return *this; } \
  ImVec4 &operator /=(const ImVec4 &v) { x /= v.x, y /= v.y, z /= v.z, w /= v.w; return *this; }

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
