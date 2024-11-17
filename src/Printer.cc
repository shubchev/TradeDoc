#include <Windows.h>
#define DEF_DEVMODE
#include "Printer.h"

class PrinterImpl : public IPrinter {
public:
};

IPrinter::IPrinter() : thread(this) {
  thread.start();
}

IPrinter::~IPrinter() {
  thread.stop();
  thread.join();
}

Printer IPrinter::create(const String &name) {
  auto pr = MakeHandle(PrinterImpl);
  if (pr) {
    pr->name = name;
  }
  return pr;
}

bool IPrinter::init() {
  if (!open()) {
    return false;
  }

  HWND hWnd = GetDesktopWindow();
  int size = DocumentProperties(NULL, handle, &name[0], NULL, NULL, 0);
  if (size <= 0) {
    close();
    return false;
  }
  propData.resize(size);

  auto &prop = getMode();
  prop.dmSize = sizeof(DEVMODE);
  prop.dmSpecVersion = DM_SPECVERSION;

  int ret = DocumentProperties(NULL, handle, &name[0], &prop, NULL, DM_OUT_BUFFER);
  if (ret != IDOK) {
    close();
    return false;
  }
  prop.dmPaperSize = DMPAPER_A4;
  prop.dmOrientation = DMORIENT_PORTRAIT;

  bool success = updateProps();
  close();

  return success;
}

bool IPrinter::open() {
  if (name.empty()) return false;
  if (handle) return true;
  return OpenPrinter(name.data(), &handle, NULL);
}

bool IPrinter::close() {
  if (!handle) return true;
  auto ret = ClosePrinter(handle);
  if (ret) handle = nullptr;
  return ret;
}

bool IPrinter::toXML(XMLElement root) const {
  if (!root) return false;

  auto &prop = getMode();
  root->SetAttribute("pageSize", prop.dmPaperSize);
  root->SetAttribute("orientation", prop.dmOrientation);
  root->SetAttribute("font", fontInfo.name);
  root->SetAttribute("fontSize", fontInfo.size);

  return true;
}

extern String defaultFont;
extern int defaultFontSize;
bool IPrinter::fromXML(XMLElement root) {
  if (!root) return false;

  auto &prop = getMode();
  prop.dmPaperSize = root->Int16Attribute("pageSize", DMPAPER_A4);
  prop.dmOrientation = root->Int16Attribute("orientation", DMORIENT_PORTRAIT);
  auto font = root->Attribute("font", defaultFont);
  auto fontSize = root->Int32Attribute("font", defaultFontSize);

  updateProps();
  setFont(font, fontSize);

  return true;
}

DEVMODE &IPrinter::getMode() {
  return *(DEVMODE *)propData.data();
}

const DEVMODE &IPrinter::getMode() const {
  return *(DEVMODE *)propData.data();
}

bool IPrinter::updateProps() {
  if (!open()) return false;
  Scope sg([&] { close(); });

  auto &prop = getMode();
  int ret = DocumentProperties(NULL, handle, &name[0],
                               &prop, &prop, DM_IN_BUFFER | DM_OUT_BUFFER);
  if (ret != IDOK) {
    return false;
  }

  sg.disable();
  return close();
}

int IPrinter::run(Thread &thread) {
  while (thread.isRunning()) {
    PrintJob job;
    if (!jobPipe.pop(job, 100)) continue;
    if (!job) continue;

    job->print();
  }
  return THREAD_SUCCESS;
}


IPrinter::FontInfo::~FontInfo() {
  for (auto &[_, f] : fonts) DeleteObject((HFONT)f);
  for (auto &[_, f] : fontsBold) DeleteObject((HFONT)f);
}

bool IPrinter::setFont(const String &name, int size) {
  char fName[32] = { 0 };
  strncpy_s(fName, name.c_str(), 31);

  Array<int> sizes;
  sizes.push_back((int)((float)size / 1.0f));
  sizes.push_back((int)((float)size / 1.5f));
  sizes.push_back((int)((float)size / 2.0f));

  FontInfo fi;
  for (auto fs : sizes) {
    auto dc = GetDC(NULL);
    auto nHeight = -MulDiv(fs, GetDeviceCaps(dc, LOGPIXELSY), 72);
    ReleaseDC(NULL, dc);

    auto font1 = CreateFont(nHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                            EASTEUROPE_CHARSET, OUT_TT_PRECIS,
                            CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE,
                            fName);
    if (!font1) {
      return false;
    }
    auto font2 = CreateFont(nHeight, 0, 0, 0, FW_BOLD, 0, 0, 0,
                            EASTEUROPE_CHARSET, OUT_TT_PRECIS,
                            CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE,
                            fName);
    if (!font2) {
      DeleteObject(font1);
      return false;
    }

    fi.fonts[fs] = font1;
    fi.fontsBold[fs] = font2;
  }

  fi.name = name;
  fi.size = size;

  std::swap(fontInfo, fi);

  return true;
}

Array<PrintJob> IPrinter::print(const Array<TradeDoc *> &docs, bool showSetup) {
  PrintJob job;
  Array<PrintJob> jobs;
  if (showSetup) {
    PRINTDLG pd;
    MemSet(&pd, 0, sizeof(pd));
    pd.lStructSize = sizeof(PRINTDLG);
    pd.hwndOwner = GetDesktopWindow();
    pd.Flags = PD_RETURNDC | PD_NONETWORKBUTTON;
    if (!PrintDlg(&pd)) {
      return {};
    }

    if (pd.hDevNames) GlobalFree(pd.hDevNames);
    if (pd.hDevMode) GlobalFree(pd.hDevMode);

    for (size_t i = 0; i < docs.size() / 2; i++) {
      auto j = 2 * i;
      auto job = IPrintJob::create(this, { docs[j], docs[j + 1] }, pd.hDC);
      jobPipe.push(job);
      jobs.push_back(job);
    }
    if (docs.size() % 2) {
      auto job = IPrintJob::create(this, { docs.back(), NULL }, pd.hDC);
      jobPipe.push(job);
      jobs.push_back(job);
    }
  } else {
    for (size_t i = 0; i < docs.size() / 2; i++) {
      auto j = 2 * i;
      auto job = IPrintJob::create(this, { docs[j], docs[j + 1] }, &getMode());
      jobPipe.push(job);
      jobs.push_back(job);
    }
    if (docs.size() % 2) {
      auto job = IPrintJob::create(this, { docs.back(), NULL }, &getMode());
      jobPipe.push(job);
      jobs.push_back(job);
    }
  }

  return jobs;
}
