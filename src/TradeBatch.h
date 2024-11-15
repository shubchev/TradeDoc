#pragma once

#include "common.h"

extern Array<const char *> unitNames;

class TradeUnit {
public:
  TradeUnit() {}
  TradeUnit(const String &_name, float _coef) : name(_name), coef(_coef) {}

  String name;
  float  coef = 1.0f;
};

class Batch {
public:
  int64_t no = 0;
  CalendarDate exp;
};


DefineHandle(IBatchTemplate, BatchTemplate);
class IBatchTemplate : public Batch {
public:
  IBatchTemplate() {}
  virtual ~IBatchTemplate() {}

  const Batch &getInfo() const { return *AS(this, const Batch); }

  // render help
  int64_t noEdit = 0;
  ImDatePicker expEdit;
  bool isEdited = false;

  void setupHelpers();
  bool isValid() const;

  static BatchTemplate create();
  static BatchTemplate create(CalendarDate exp);
  static BatchTemplate create(int64_t batch, CalendarDate exp);
};




