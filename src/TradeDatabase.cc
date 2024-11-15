#include <Windows.h>
#define DEF_DEVMODE
#include "TradeDatabase.h"
#include <inttypes.h>

extern bool fromString(const String &date, CalendarDate &cd);

TradeDB::TradeDB() {
  docNumLeadZero = MakeHandle(int, 9);
  batchLeadZero = MakeHandle(int, 6);
}

void TradeDB::add(TradeDoc &doc) {
  docs[doc.date].push_back(std::move(doc));
}

int64_t TradeDB::getMaxDocNo() const {
  int64_t no = 0;
  for (auto &[_, d] : docs) {
    for (auto &td : d) no = max(no, td.no);
  }
  return no;
}

bool TradeDB::hasDocNo(int64_t no) const {
  for (auto &[_, d] : docs) {
    for (auto &td : d) if (td.no == no) return true;
  }
  return false;
}

bool TradeDB::loadDocs() {
  lastError.clear();
  auto docsDir = dir / "docs";
  auto findRes = find(docsDir, SRCH_FILES | SRCH_RECURSIVE, "*.xml");

  Map<CalendarDate, Array<TradeDoc>> tmpDocs;
  for (auto &fe : findRes) {
    TradeDoc td;
    XMLDocument xml;
    if (xml.Load(docsDir / fe.path) != XMLError::Ok) {
      lastError = "Failed to load document " + fe.path.string();
      return false;
    }

    if (xml.Root()->Name() != "Document") {
      lastError = "Malformed document " + fe.path.string();
      return false;
    }

    if (!td.fromXML(xml.Root())) {
      lastError = "Failed to parse document " + fe.path.string();
      return false;
    }
    tmpDocs[td.date].push_back(std::move(td));
  }

  std::swap(docs, tmpDocs);

  return true;
}

bool TradeDB::saveDoc(const TradeDoc &doc) const {
  XMLDocument xml("Document");
  if (!doc.toXML(xml.Root())) {
    lastError = "Failed to create document " + toString(doc.no);
    return false;
  }

  auto cPath = dir / "docs";
  cPath /= toString(doc.getY());
  cPath /= toString(doc.getM());
  cPath /= toString(doc.getD());
  if (!createDirectories(cPath)) {
    lastError = "Failed to create document path " + cPath.string();
    return false;
  }

  char name[32] = { 0 };
  SPRINTF(name, "%09" PRId64 ".xml", doc.no);
  cPath /= name;

  if (xml.Save(cPath) != XMLError::Ok) {
    lastError = "Failed to save document " + toString(doc.no);
    ::remove(cPath);
    return false;
  }

  return true;
}

bool TradeDB::saveBatches() const {
  XMLDocument xml("Batches");
  lastError.clear();

  auto root = xml.Root();
  if (!root) {
    lastError = "Failed to allocate batch information";
    return false;
  }

  for (auto &b : batchTemplate) {
    auto c = root->NewChild("Batch");
    if (!c) {
      lastError = "Failed to allocate batch information entry";
      return false;
    }
    c->SetAttribute("no", b->noEdit);
    c->SetAttribute("exp", b->expEdit.getSelection().str(DateFormat::DMY));
  }

  if (xml.Save(dir / "batches.xml") != XMLError::Ok) {
    lastError = "Failed to save batch information";
    return false;
  }

  for (auto &b : batchTemplate) {
    b->no = b->noEdit;
    b->exp = b->expEdit.getSelection();
  }

  return true;
}

bool TradeDB::loadBatches() {
  XMLDocument xml;
  lastError.clear();
  if (xml.Load(dir / "batches.xml") != XMLError::Ok) {
    lastError = "Failed to load batch information";
    return false;
  }

  auto root = xml.Root();
  if (!root) {
    lastError = "Failed to allocate batch information";
    return false;
  }
  if (root->Name() != "Batches") {
    lastError = "Malformed batch information";
    return false;
  }

  Array<BatchTemplate> tmpBatch;
  for (auto c = root->FirstChild("Batch"); c; c = c->NextSibling("Batch")) {
    bool ret = true;
    auto no = c->Int64Attribute("no", -1); if (no < 0) ret = false;
    auto exp = c->Attribute("exp"); if (exp.empty()) ret = false;

    auto b = IBatchTemplate::create();
    if (!b) {
      lastError = "Failed to create batch entry";
      return false;
    }

    b->no = no;
    if (!fromString(exp, b->exp)) ret = false;
    if (!ret) {
      lastError = "Failed to parse batch entry";
      return false;
    }

    b->setupHelpers();
    tmpBatch.push_back(b);
  }

  std::swap(batchTemplate, tmpBatch);

  return true;
}

bool TradeDB::saveProducts() const {
  XMLDocument xml("Products");
  lastError.clear();

  auto root = xml.Root();
  if (!root) {
    lastError = "Failed to allocate products information";
    return false;
  }

  for (auto &p : productTemplate) {
    auto c = root->NewChild("Product");
    if (!c) {
      lastError = "Failed to allocate products information entry";
      return false;
    }
    if (!p->toXML(c)) {
      lastError = "Failed to write products information entry";
      return false;
    }
  }

  if (xml.Save(dir / "products.xml") != XMLError::Ok) {
    lastError = "Failed to save products information";
    return false;
  }

  for (auto &p : productTemplate) {
    auto b = p->batchEdit.lock();
    p->name = p->nameEdit.getText();
    if (b) p->batch = b->getInfo();
    else p->batch = Batch();
    p->unit.name = unitNames[p->unitNameEdit];
    p->unit.coef = p->unitCoefEdit;
  }

  return true;
}

bool TradeDB::loadProducts() {
  XMLDocument xml;
  lastError.clear();
  if (xml.Load(dir / "products.xml") != XMLError::Ok) {
    lastError = "Failed to load products information";
    return false;
  }

  auto root = xml.Root();
  if (!root) {
    lastError = "Failed to allocate products information";
    return false;
  }
  if (root->Name() != "Products") {
    lastError = "Malformed products information";
    return false;
  }

  Array<ProductTemplate> tmpProd;
  for (auto c = root->FirstChild("Product"); c; c = c->NextSibling("Product")) {
    auto p = IProductTemplate::create();
    if (!p) {
      lastError = "Failed to create product information entry";
      return false;
    }
    if (!p->fromXML(*this, c)) {
      lastError = "Failed to parse product information entry";
      return false;
    }
    tmpProd.push_back(std::move(p));
  }

  std::swap(productTemplate, tmpProd);

  return true;
}

bool TradeDB::saveInfo() const {
  XMLDocument xml("Info");
  lastError.clear();

  auto root = xml.Root();
  if (!root) return true;

  auto c = root->NewChild("DocNumLeadZero");
  if (c) c->SetText(*docNumLeadZero);

  c = root->NewChild("BatchLeadZero");
  if (c) c->SetText(*batchLeadZero);

  c = root->NewChild("Printers");
  if (c) printerSettings.toXML(c);

  if (xml.Save(dir / "info.xml") != XMLError::Ok) {
    lastError = "Failed to save extra information";
    return false;
  }
  return true;
}

bool TradeDB::loadInfo() {
  XMLDocument xml;
  lastError.clear();
  if (xml.Load(dir / "info.xml") != XMLError::Ok) {
    lastError = "Failed to load extra information";
    return false;
  }

  auto root = xml.Root();
  if (!root) return true;
  if (root->Name() != "Info") return true;

  auto c = root->FirstChild("DocNumLeadZero");
  if (c) *docNumLeadZero = c->IntText(*docNumLeadZero);

  c = root->FirstChild("BatchLeadZero");
  if (c) *batchLeadZero = c->IntText(*batchLeadZero);

  c = root->FirstChild("Printers");
  if (c) printerSettings.fromXML(c);

  return true;
}

bool TradeDB::loadAll() {
  bool ret = loadInfo();
  ret = ret && loadBatches();
  ret = ret && loadProducts();
  ret = ret && loadDocs();
  return ret;
}

bool TradeDB::isOpen() const {
  return exist(dir);
}

bool TradeDB::open(const Path &path) {
  bool hasZip = exist(path);
  bool hasDB = exist(dir);

  if (!hasZip && !hasDB) {
    // open new DB, archive will be created on exit
    if (!createDirectories(dir)) {
      lastError = "Failed to extract database";
      return false;
    }
    {
      OutputFile fs(dir / "info.xml");
      if (fs) fs << "<Info/>";
    }
    {
      OutputFile fs(dir / "products.xml");
      if (fs) fs << "<Products/>";
    }
    {
      OutputFile fs(dir / "batches.xml");
      if (fs) fs << "<Batches/>";
    }

    auto attrib = GetFileAttributes(dir.string().c_str());
    SetFileAttributes(dir.string().c_str(), attrib | FILE_ATTRIBUTE_HIDDEN);
  } else if (!hasZip && hasDB) {
    // DB already open, archive will be created on exit
  } else if (hasZip && !hasDB) {
    // extract DB from archive, archive will be updated on exit
    fz = IFileZip::open(path);
    if (!fz) {
      lastError = "Not a valid database archive";
      return false;
    }
    if (!fz->extract("", false)) {
      lastError = "Failed to extract database";
      return false;
    }

    auto attrib = GetFileAttributes(dir.string().c_str());
    SetFileAttributes(dir.string().c_str(), attrib | FILE_ATTRIBUTE_HIDDEN);
  } else {
    // DB already open, archive will be updated on exit
  }

  archivePath = path;

  return true;
}

bool TradeDB::close() {
  fz = nullptr;

  Timestamp lastTS = getFileLastWriteTime(archivePath);
  Timestamp newTS = lastTS;
  do {
    if (!IFileZip::archive(".db", archivePath, false)) {
      lastError = "Failed to archive database";
      return false;
    }
    Thread::sleep(100);
    newTS = getFileLastWriteTime(archivePath);
  } while (newTS == lastTS);

  return true;
}