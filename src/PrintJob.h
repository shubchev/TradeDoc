#pragma once

#include "common.h"
#include "TradeDocument.h"

#ifndef DEF_DEVMODE
struct DEVMODE;
#endif

class IPrinter;
DefineHandle(IPrintJob, PrintJob);
class IPrintJob {
public:
  virtual ~IPrintJob() {}


  virtual int id() const = 0;
  virtual bool isDone() const = 0;
  virtual void cancel() = 0;
  virtual bool wait(int timeout) = 0;
  virtual void print() = 0;

  static PrintJob create(IPrinter *pr, Pair<TradeDoc *, TradeDoc *> docs, const DEVMODE *props);
  static PrintJob create(IPrinter *pr, Pair<TradeDoc *, TradeDoc *> docs, void *hDC);
};
