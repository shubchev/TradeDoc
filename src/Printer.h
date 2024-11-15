#pragma once

#include "PrintJob.h"

DefineHandle(IPrinter, Printer);
class IPrinter : protected Thread::IRunnable {
  friend class PrintJobImpl;

public:
  class FontInfo {
  public:
    ~FontInfo();
    Map<int, void *> fonts;
    Map<int, void *> fontsBold;
    int size = 0;
    String name;
  };
  FontInfo fontInfo;

protected:
  Thread thread;
  Pipe<PrintJob> jobPipe;
  int run(Thread &thread) override;

  String name;
  Array<uint8_t> propData;
  void *handle = nullptr;

  bool open();
  bool close();

  IPrinter();

public:
  virtual ~IPrinter();

  Handle<int> batchLeadZero;
  Handle<int> docNumLeadZero;

  int getFontSize() const { return fontInfo.size; }
  const String &getFontName() const { return fontInfo.name; }
  bool setFont(const String &font, int size);

  const String &getName() const { return name; }
  DEVMODE &getMode();
  const DEVMODE &getMode() const;

  bool init();
  bool updateProps();

  bool toXML(XMLElement root) const;
  bool fromXML(XMLElement root);

  Array<PrintJob> print(const Array<TradeDoc *> &docs, bool showSetup);

  static Printer create(const String &name);
};