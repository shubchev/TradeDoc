#pragma once

#include "TradeProduct.h"

class TradeDoc {
  friend class TradeDB;
public:
  TradeDoc() {}

  Array<Product> products;
  String recipient;
  int64_t no = 0;
  CalendarDate date;

  int getY() const { return date.year(); }
  int getM() const { return date.month(); }
  int getD() const { return date.day(); }

  bool toXML(XMLElement root) const;
  bool fromXML(XMLElement root);
};