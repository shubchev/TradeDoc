#pragma once

#include <Windows.h>
#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "resource.h"


extern HINSTANCE g_hInst;

#define DefineHandle(T, Tname) \
  class T; \
  typedef std::shared_ptr<T> Tname; \
  typedef std::weak_ptr<T> Tname##Ref

#define MakeHandle(Timpl, ...) std::make_shared<Timpl>(__VA_ARGS__)