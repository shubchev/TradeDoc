#pragma once

#include "common.h"
#include "WinSubclass.h"
#include "Label.h"
#include "TextBox.h"
#include "DatePicker.h"

class TradeDocProduct {
public:
  std::wstring name;
  float weight = -1.0f;
  int64_t batchNo = -1;
  SYSTEMTIME expirity;
};

class TradeDoc {
protected:
  std::vector<TradeDocProduct> products;

public:

  
  void deinit();
};

