#pragma once

#include "TradeBatch.h"

class Product {
public:
  String name;
  TradeUnit unit;
  Batch batch;
  int quantity = 0;

  virtual float weight() const;
  virtual float weight(int quantity) const;
  virtual bool toXML(XMLElement root) const;
  virtual bool fromXML(XMLElement root);
};

class TradeDB;
DefineHandle(IProductTemplate, ProductTemplate);
class IProductTemplate : public Product {
public:
  IProductTemplate() {}
  virtual ~IProductTemplate() {}

  float weight(int quantity) const override;
  bool toXML(XMLElement root) const override;
  bool fromXML(TradeDB &db, XMLElement root);

  // helpers
  ImTextField nameEdit;
  BatchTemplateRef batchEdit;
  int unitNameEdit = 0;
  float unitCoefEdit = 1.0f;
  bool isNew = false;

  void setupHelpers(TradeDB &db);
  bool isEdited() const;
  bool isValid() const;

  static ProductTemplate create();
  static ProductTemplate create(const String &name, const TradeUnit unit);
};
