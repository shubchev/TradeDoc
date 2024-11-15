#include <Windows.h>
#define DEF_DEVMODE
#include "PrinterSettings.h"

String defaultFont = "Courier New";
int defaultFontSize = 80;

int CALLBACK EnumFontsProc(
  _In_ const LOGFONT *lf,
  _In_ const TEXTMETRIC *tm,
  _In_       DWORD      dwType,
  _In_       LPARAM     lParam) {
  auto ps = (PrinterSettings *)lParam;
  if (lf->lfFaceName[0] == '@') return 1;
  String fontName = (char *)lf->lfFaceName;
  ps->fontNameIndex[fontName] = (int)ps->fontNames.size();
  ps->fontNames.push_back(fontName);
  ps->fontNamesList += fontName;
  ps->fontNamesList.push_back(0);
  return 1;
}


bool PrinterSettings::init() {
  auto dc = GetDC(NULL);
  EnumFonts(dc, NULL, EnumFontsProc, (LPARAM)this);
  ReleaseDC(NULL, dc);

  String defFontName;
  int defFontSize = 128;
  if (!fontNameIndex.count("Courier New") && fontNameIndex.count("Ariel")) {
    fontNames.clear();
    fontNamesList.clear();
    fontNameIndex.clear();
  } else if (fontNameIndex.count("Courier New")) {
    defFontName = "Courier New";
  } else if (fontNameIndex.count("Ariel")) {
    defFontName = "Ariel";
  }

  char defaultPrinterName[MAX_PATH];
  DWORD cchPrinter(ARRAYSIZE(defaultPrinterName));

  GetDefaultPrinter(defaultPrinterName, &cchPrinter);

  DWORD dwNeeded = 0, dwReturned = 0;
  auto fnReturn = EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                               NULL,
                               1L,                // printer info level
                               (LPBYTE)NULL, 0L, &dwNeeded, &dwReturned);

  Array<PRINTER_INFO_1> info(dwNeeded);
  if (!info.empty()) {
    dwReturned = 0;
    fnReturn = EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                            NULL,
                            1L,                // printer info level
                            (LPBYTE)info.data(),
                            dwNeeded, &dwNeeded, &dwReturned);
  }

  for (DWORD i = 0; i < dwReturned; i++) {
    auto p = IPrinter::create(info[i].pName);
    if (!p) continue;
    p->batchLeadZero = batchLeadZero;
    p->docNumLeadZero = docNumLeadZero;

    if (!p->init()) continue;
    if (!p->setFont(defFontName, defFontSize)) continue;

    if (p->getName() == defaultPrinterName) {
      printerIndex = i;
      defaultPrinter = i;
    }
    printers.push_back(p);
  }


  return !printers.empty();
}

bool PrinterSettings::toXML(XMLElement root) const {
  if (!root) return true;

  for (size_t i = 0; i < printers.size(); i++) {
    auto &pr = printers[i];
    auto c = root->NewChild("Printer");
    if (c) {
      pr->toXML(c);
      c->SetAttribute("name", pr->getName());
      if (i == defaultPrinter) c->SetAttribute("default", true);
    }
  }

  return true;
}

bool PrinterSettings::fromXML(XMLElement root) {
  if (!root) return true;

  for (auto c = root->FirstChild("Printer"); c; c = c->NextSibling("Printer")) {
    auto name = c->Attribute("name");
    if (name.empty()) continue;

    Printer pr;
    for (auto &p : printers) {
      if (p->getName() == name) { pr = p; break; }
    }

    if (pr) {
      pr->fromXML(c);
    }
  }

  return true;
}

Array<PrintJob> PrinterSettings::print(const Array<TradeDoc *> &docs, bool showSetup) {
  if (printerIndex < 0 || printerIndex >= (int)printers.size()) {
    return {};
  }
  return printers[printerIndex]->print(docs, showSetup);
}

void PrinterSettings::openSettings() {
  opened = true;
}

void PrinterSettings::closeSettings() {
  opened = false;
}

void PrinterSettings::renderSettings() {
  if (!opened) {
    return;
  }

  if (!ImGui::Begin("Print settings", &opened,
                    ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::End();
    return;
  }

  {
    ImGui::Text("Printer   ");
    ImGui::SameLine();
    String prntItem;
    for (auto &p : printers) {
      prntItem += p->getName();
      prntItem.push_back(0);
    }
    prntItem.push_back(0);
    if (ImGui::Combo("##printerSelect", &printerIndex, prntItem.c_str())) {

    }
  }

  auto &pi = printers[printerIndex];
  auto &prop = pi->getMode();
  bool changeMade = false;

  if (prop.dmFields & DM_PAPERSIZE) {
    ImGui::Separator();
    ImGui::Text("Paper size");
    ImGui::SameLine();
    const char letterItem[] =
      "Letter\0"
      "A4\0"
      "\0";

    int pageSizeIndex = 0;
    switch (prop.dmPaperSize) {
      case DMPAPER_LETTER: pageSizeIndex = 0; break;
      case DMPAPER_A4: pageSizeIndex = 1; break;
      default: pageSizeIndex = 1; break;
    }

    if (ImGui::Combo("##letterSelect", &pageSizeIndex, letterItem)) {
      switch (pageSizeIndex) {
        case 0: prop.dmPaperSize = DMPAPER_LETTER; break;
        case 1: prop.dmPaperSize = DMPAPER_A4; break;
        default: prop.dmPaperSize = DMPAPER_A4; break;
      }
      changeMade = true;
    }
  }

  if (prop.dmFields & DM_ORIENTATION) {
    ImGui::Separator();
    ImGui::Text("Orientation");
    ImGui::Indent();
    if (ImGui::RadioButton("Landscape", prop.dmOrientation == DMORIENT_LANDSCAPE)) {
      prop.dmOrientation = DMORIENT_LANDSCAPE;
      changeMade = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Portrait", prop.dmOrientation == DMORIENT_PORTRAIT)) {
      prop.dmOrientation = DMORIENT_PORTRAIT;
      changeMade = true;
    }
    ImGui::Unindent();
  }

  {
    ImGui::Separator();
    ImGui::Text("Font");

    auto &fontName = pi->getFontName();
    auto fontNameIdx = fontNameIndex[fontName];
    if (ImGui::Combo("##fontSelect", &fontNameIdx, fontNamesList.c_str())) {

    }


    static Array<int> fontSizes;
    static Map<int, int> fontSizesMap;
    static String fontSizesList;
    if (fontSizes.empty()) {
      for (int fs = 8, i = 0; fs <= 144; fs += 2, i++) {
        fontSizes.push_back(fs);
        fontSizesMap[fs] = i;
        fontSizesList += toString(fs);
        fontSizesList.push_back(0);
      }
    }

    int fontSize = pi->getFontSize();
    int fontSizeIdx = fontSizesMap[fontSize];
    if (ImGui::Combo("##fontSizeSelect", &fontSizeIdx, fontSizesList.c_str())) {
      auto cFont = std::move(pi->fontInfo);
      if (!pi->setFont(fontNames[fontNameIdx], fontSizes[fontSizeIdx])) {
        pi->fontInfo = std::move(cFont);
      }
    }

    ImGui::Unindent();
  }

  ImGui::End();

  if (changeMade) {
    pi->updateProps();
  }
}