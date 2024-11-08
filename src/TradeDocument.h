#pragma once

#include "common.h"

class TradeDocProduct {
public:
  String name;
  float weight = -1.0f;
  int64_t batchNo = -1;
  CalendarTime expirity;
};

class TradeDoc {
public:
  CalendarTime date;
  Array<TradeDocProduct> products;
  int64_t no = 0;
  String recipient;

  int getY() const { return date.year; }
  int getM() const { return date.month; }
  int getD() const { return date.day; }
};


class TradeDB {
public:
  Map<int, Map<int, Map<int, Map<int64_t, TradeDoc>>>> db;

  void add(const TradeDoc &doc);
};

