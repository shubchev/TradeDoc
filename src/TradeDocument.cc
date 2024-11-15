#include "TradeDocument.h"

extern bool fromString(const String &date, CalendarDate &cd);

bool TradeDoc::toXML(XMLElement root) const {
  if (!root) return false;

  auto n = root->NewChild("No"); if (!n) return false;
  auto t = root->NewChild("To"); if (!t) return false;
  auto d = root->NewChild("Date"); if (!d) return false;

  n->SetText(no);
  t->SetText(recipient);
  d->SetText(date.str(DateFormat::DMY));

  for (auto &p : products) {
    auto c = root->NewChild("Product");
    if (!p.toXML(c)) return false;
  }

  return true;
}

bool TradeDoc::fromXML(XMLElement root) {
  if (!root) return false;

  auto n = root->FirstChild("No"); if (!n) return false;
  auto t = root->FirstChild("To"); if (!t) return false;
  auto d = root->FirstChild("Date"); if (!d) return false;
  
  auto tNo = n->Int64Text(-1); if (tNo < 0) return false;
  auto tRe = t->Text(); if (tRe.empty()) return false;
  auto tDate = d->Text(); if (tDate.empty()) return false;

  Array<Product> tmpP;
  for (auto c = root->FirstChild("Product"); c; c = c->NextSibling("Product")) {
    Product p;
    if (!p.fromXML(c)) return false;
    tmpP.push_back(std::move(p));
  }

  if (!fromString(tDate, date)) return false;
  no = tNo;
  std::swap(recipient, tRe);
  std::swap(products, tmpP);

  return true;
}