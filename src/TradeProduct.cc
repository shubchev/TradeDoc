#include "TradeProduct.h"
#include "TradeDatabase.h"

bool fromString(const String &date, CalendarDate &cd) {
  if (date.empty()) return false;
  int y = 0, m = 0, d = 0;
  if (SSCANF(date.c_str(), "%d-%d-%d", &d, &m, &y) != 3) return false;
  try { cd = CalendarDate(d, m, y); } catch (Exception &) { return false; }
  return true;
}




float Product::weight() const {
  return unit.coef * quantity;
}

float Product::weight(int q) const {
  return unit.coef * q;
}

bool Product::toXML(XMLElement root) const {
  if (!root) return false;

  TradeUnit tUnit;
  auto n = root->NewChild("Name"); if (!n) return false;
  auto u = root->NewChild("Unit"); if (!u) return false;
  auto b = root->NewChild("Batch"); if (!b) return false;
  auto q = root->NewChild("Quantity"); if (!q) return false;

  n->SetText(name);
  b->SetAttribute("no", batch.no);
  b->SetAttribute("exp", batch.exp.str(DateFormat::DMY));
  u->SetAttribute("name", unit.name);
  u->SetAttribute("coef", unit.coef);
  q->SetText(quantity);

  return true;
}

bool Product::fromXML(XMLElement root) {
  if (!root) return false;

  auto n = root->FirstChild("Name"); if (!n) return false;
  auto u = root->FirstChild("Unit"); if (!u) return false;
  auto b = root->FirstChild("Batch"); if (!b) return false;
  auto q = root->FirstChild("Quantity"); if (!q) return false;

  auto tName = n->Text(); if (tName.empty()) return false;
  auto tQuan = q->Int32Text(-1); if (tQuan < 0) return false;
  auto tUnit = u->Attribute("name"); if (tUnit.empty()) return false;
  auto tCoef = u->FloatAttribute("coef", -1.0f); if (tCoef <= 0.0f) return false;
  auto tNo = b->Int64Attribute("no", -1.0f); if (tNo < 0) return false;
  auto tExp = b->Attribute("exp"); if (tExp.empty()) return false;

  if (!fromString(tExp, batch.exp)) return false;
  std::swap(name, tName);
  std::swap(unit.name, tUnit);
  unit.coef = tCoef;
  batch.no = tNo;
  quantity = tQuan;

  return true;
}



bool IProductTemplate::toXML(XMLElement root) const {
  if (!root) return false;

  if (unitCoefEdit <= 0.0f) return false;
  if (unitNameEdit < 0) return false;
  auto bt = batchEdit.lock();
  auto n = root->NewChild("Name"); if (!n) return false;
  auto u = root->NewChild("Unit"); if (!u) return false;

  n->SetText(nameEdit.getText());
  u->SetAttribute("name", unitNames[unitNameEdit]);
  u->SetAttribute("coef", unitCoefEdit);
  if (bt) {
    auto b = root->NewChild("Batch"); if (!b) return false;
    b->SetAttribute("no", bt->no);
    b->SetAttribute("exp", bt->exp.str(DateFormat::DMY));
  }

  return true;
}
bool IProductTemplate::fromXML(TradeDB &db, XMLElement root) {
  if (!root) return false;

  auto n = root->FirstChild("Name"); if (!n) return false;
  auto u = root->FirstChild("Unit"); if (!u) return false;
  auto b = root->FirstChild("Batch");

  auto tName = n->Text(); if (tName.empty()) return false;
  auto tUnit = u->Attribute("name"); if (tUnit.empty()) return false;
  auto tCoef = u->FloatAttribute("coef", -1.0f); if (tCoef <= 0.0f) return false;
  if (b) {
    auto tNo = b->Int64Attribute("no", -1); if (tNo < 0) return false;
    auto tExp = b->Attribute("exp"); if (tExp.empty()) return false;

    if (!fromString(tExp, batch.exp)) return false;
    batch.no = tNo;
  }
  std::swap(name, tName);
  std::swap(unit.name, tUnit);
  unit.coef = tCoef;

  setupHelpers(db);

  return true;
}

float IProductTemplate::weight(int q) const {
  return unitCoefEdit * q;
}

void IProductTemplate::setupHelpers(TradeDB &db) {
  nameEdit.setText(name);
  batchEdit.reset();
  for (auto &b : db.batchTemplate) {
    if (b->no == batch.no) { batchEdit = b; break; }
  }
  unitCoefEdit = unit.coef;
  unitNameEdit = 0;
  for (auto &n : unitNames) {
    if (unit.name == n) break;
    unitNameEdit++;
  }
  if (unitNameEdit == (int)unitNames.size()) unitNameEdit = 0;
}

bool IProductTemplate::isEdited() const {
  if (unitCoefEdit != unit.coef) return true;
  if (unitNames[unitNameEdit] != unit.name) return true;
  if (isNew && nameEdit.getText() != name) return true;
  auto b = batchEdit.lock();
  if (b && b->no != batch.no) return true;

  return false;
}

bool IProductTemplate::isValid() const {
  if (unitCoefEdit <= 0.0f) return false;
  if (unitNameEdit < 0 || unitNameEdit >= (int)unitNames.size()) return false;
  if (isNew && nameEdit.getText().empty()) return false;
  return true;
}

ProductTemplate IProductTemplate::create() {
  return create(String(), TradeUnit());
}

ProductTemplate IProductTemplate::create(const String &name,
                                         const TradeUnit unit) {
  auto pt = MakeHandle(IProductTemplate);
  if (pt) {
    pt->name = name;
    pt->unit = unit;
  }
  return pt;
}