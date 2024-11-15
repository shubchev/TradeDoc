#pragma once

#include "TradeDocument.h"
#include "PrinterSettings.h"

class TradeDB {
public:
  TradeDB();

  Array<BatchTemplate> batchTemplate;
  Array<ProductTemplate> productTemplate;

  // year -> month -> day -> name -> doc
  Map<CalendarDate, Array<TradeDoc>> docs;

  Path dir = ".db";
  FileZip fz;
  Path archivePath;
  mutable String lastError;


  // new doc
  CalendarDate docDate;
  String docReceiver;
  int64_t docNumber = 0;
  Handle<int> docNumLeadZero;
  Handle<int> batchLeadZero;

  // printing
  PrinterSettings printerSettings;

  void add(TradeDoc &doc);
  int64_t getMaxDocNo() const;
  bool hasDocNo(int64_t no) const;

  bool loadDocs();
  bool saveDoc(const TradeDoc &doc) const;

  bool saveBatches() const;
  bool loadBatches();

  bool saveProducts() const;
  bool loadProducts();

  bool saveInfo() const;
  bool loadInfo();

  bool loadAll();

  bool isOpen() const;
  bool open(const Path &path);
  bool close();
};

