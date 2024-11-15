#include "TradeBatch.h"

Array<const char *> unitNames = {
  u8"л",
  u8"кг",
};


void IBatchTemplate::setupHelpers() {
  noEdit = no;
  expEdit.setCurrentDate(exp);
}

bool IBatchTemplate::isValid() const {
  if (noEdit < 0) return false;
  if (expEdit.getSelection() <= CalendarDate::today()) return false;
  return true;
}

BatchTemplate IBatchTemplate::create() {
  return create(0, CalendarDate::today());
}
BatchTemplate IBatchTemplate::create(CalendarDate exp) {
  return create(0, exp);
}
BatchTemplate IBatchTemplate::create(int64_t batch, CalendarDate exp) {
  auto tb = MakeHandle(IBatchTemplate);
  if (tb) {
    tb->no = batch;
    tb->exp =exp;
  }
  return tb;
}