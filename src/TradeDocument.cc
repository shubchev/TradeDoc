#include "TradeDocument.h"





void TradeDB::add(const TradeDoc &doc) {
  db[doc.getY()][doc.getM()][doc.getD()][doc.no] = doc;
}