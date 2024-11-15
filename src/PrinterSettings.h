#pragma once

#include "Printer.h"

class PrinterSettings {
public:
  Array<String> fontNames;
  String fontNamesList;
  Map<String, int> fontNameIndex;

  Handle<int> batchLeadZero;
  Handle<int> docNumLeadZero;

protected:
  Array<Printer> printers;
  int defaultPrinter = 0;
  int printerIndex = 0;

  // render members
  bool opened = false;

public:
  bool init();

  Array<PrintJob> print(const Array<TradeDoc *> &docs, bool showSetup);

  bool toXML(XMLElement root) const;
  bool fromXML(XMLElement root);

  void openSettings();
  void closeSettings();
  void renderSettings();
};