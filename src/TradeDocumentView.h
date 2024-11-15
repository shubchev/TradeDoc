#pragma once

#include "TradeDatabase.h"
#include "PrinterSettings.h"

class TradeDocView {
protected:
  TradeDB *db = nullptr;
  TradeDoc mDoc;
  Array<String> *columnInfo = nullptr;
  uint32_t dockNextTo = 0;
  char title[32] = { 0 };
  bool firstOpenRenderView = true;
  bool openFlag = false;
  bool focusFlag = false;

public:
  TradeDocView();
  TradeDocView(TradeDB &db, TradeDoc &doc,
               uint32_t dockNextTo, Array<String> *columnInfo);

  const TradeDoc &doc() const { return mDoc; }

  void focus();
  void close();
  bool render();
  bool isOpen() const { return openFlag; }
};