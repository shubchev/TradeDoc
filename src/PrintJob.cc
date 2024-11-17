#include <Windows.h>
#define DEF_DEVMODE
#include "PrintJob.h"
#include "Printer.h"

class PrintJobImpl : public IPrintJob {
public:
  Mutex lock;
  Condition cond;

  IPrinter *printer = nullptr;
  HDC hDC = NULL;
  const DEVMODE *pProp = nullptr;
  ivec2 imgSize = ivec2(0);
  vec2 px = vec2(0);
  Array<HFONT> font;
  Array<HFONT> fontBold;
  Map<uint32_t, HBRUSH> brush;
  Map<uint32_t, HPEN> pens;
  bool isLandscape = false;
  bool printToFile = false;


  TradeDoc *pDoc[2] = { nullptr, nullptr };
  int jobId = -1;
  bool done = false;

  int id() const override { return jobId; }
  bool isDone() const override { return done; }
  void cancel() override {

  }
  bool wait(int timeout) override {
    UniqueLock ul(lock);
    if (done) return true;
    return cond.wait_for(ul, Milliseconds(timeout), [&] { return jobId <= 0; });
  }

  void print() override;

  inline vec2 pixToUV(const ivec2 &p) {
    return vec2((float)p.x / imgSize.x, (float)p.y / imgSize.y);
  }
  inline ivec2 uvToPix(const vec2 &uv) {
    return ivec2(uv.x * imgSize.x, uv.y * imgSize.y);
  }

  bool startDoc(const WString &name);
  void renderDoc(TradeDoc *doc, int idx);

  class TableCellInfo {
  public:
    TableCellInfo() {}
    TableCellInfo(PrintJobImpl *pj, const WString &s, HFONT f, int from)
      : parent(pj), str(s), font(f), startFrom(from){
      size = pj->calcText(str, f);
    }
    PrintJobImpl *parent = nullptr;
    WString str;
    HFONT font = NULL;
    vec2 pos = vec2(0);
    vec2 size = vec2(0);
    int startFrom = 0;
    uint32_t color = RGB(0, 0, 0);

    // startFrom: 0 - top left, 1 - top center, 2 - top right
    // startFrom: 3 - bot left, 4 - bot center, 5 - bot right
    void draw() {
      SelectObject(parent->hDC, font);
      parent->drawText(color, pos, size, str);
    }

    void calcXPos() {
      if (startFrom == 0 || startFrom == 3) pos.x = pos.x;
      else if (startFrom == 1 || startFrom == 4) pos.x -= size.x * 0.5f;
      else pos.x -= size.x;
    }
    void calcYPos() {
      if (startFrom < 3) pos.y = pos.y;
      else pos.y -= size.y;
    }

    void calcXPos(float wMargin, float wSize) {
      if (startFrom == 0 || startFrom == 3) {
        pos.x = wMargin;
      } else if (startFrom == 1 || startFrom == 4) {
        pos.x = wMargin + (wSize - size.x) * 0.5f;
      } else {
        pos.x = wMargin + (wSize - size.x);
      }
    }
    void calcYPos(float hMargin, float hSize) {
      if (startFrom < 3) pos.y = hMargin + (hSize - size.y) * 0.5f;
      else pos.y = hMargin + hSize - (hSize - size.y) * 0.5f;
    }
  };

  #define companyStr  extraStrings[0]
  #define docTypeStr  extraStrings[1]
  #define docNoStr    extraStrings[2]
  #define dateStr     extraStrings[3]
  #define toStr       extraStrings[4]
  #define noteStr     extraStrings[5]
  #define mpsStr      extraStrings[6]
  #define issuerStr   extraStrings[7]
  Array<TableCellInfo> extraStrings;
  Array<Array<TableCellInfo>> tableStrings;
  Array<float> tableWidthMargins;
  Array<float> tableHeightMargins;

  void genTableStrings(TradeDoc *pDoc, vec4 rect) {
    float topMargin = rect.x;
    float leftMargin = rect.y;
    float rightMargin = rect.z;
    float bottomMargin = rect.w;

    extraStrings.clear();
    tableStrings.clear();
    tableWidthMargins.clear();
    tableHeightMargins.clear();

#define ADD_EXTRA_STR(str, fnt, from, ...) \
    extraStrings.push_back(TableCellInfo(this, str, fnt, from)); \
    extraStrings.back().pos = vec2(__VA_ARGS__); \
    extraStrings.back().calcXPos(); \
    extraStrings.back().calcYPos()


    ADD_EXTRA_STR(L"СИЛТОНА ЕООД", fontBold[2], 1,
                  0.5f, topMargin);
    ADD_EXTRA_STR(L"ТЪРГОВСКИ ДОКУМЕНТ", fontBold[2], 1,
                  0.5f, topMargin + companyStr.size.y * 1.5f);

    wchar_t noFmt[32], tmp[32];
    SWPRINTF(tmp, L"№  %%0%d" PRId64, *printer->docNumLeadZero);
    SWPRINTF(noFmt, tmp, pDoc->no);
    ADD_EXTRA_STR(noFmt, fontBold[2], 0,
                  leftMargin, docTypeStr.pos.y + docTypeStr.size.y * 1.5f);

    auto docDate = toWString(u8"дата: " + pDoc->date.str(DateFormat::DMY, "."));
    ADD_EXTRA_STR(docDate, fontBold[1], 2,
                  rightMargin, docTypeStr.pos.y + docTypeStr.size.y * 1.5f);

    auto to = toWString(u8"Получател: " + pDoc->recipient);
    ADD_EXTRA_STR(to, fontBold[1], 2,
                  rightMargin, dateStr.pos.y + dateStr.size.y * 1.5f);






    const Array<WString> columnName = {
      toWString(u8"№"),
      toWString(u8"Продукт"),
      toWString(u8"Тегло"),
      toWString(u8"№ на партида"),
      toWString(u8"Произход"),
      toWString(u8"Годност"),
    };
    int Nx = (int)columnName.size();
    int Ny = max((int)pDoc->products.size(), 12);

    int weightLeadDigit[2] = { 0, 0 };
    for (auto &p : pDoc->products) {
      int d = 1, w = (int)p.unit.coef;
      if (w) d--;
      while (w) { d++; w /= 10; }
      weightLeadDigit[0] = max(weightLeadDigit[0], d);
    }
    for (auto &p : pDoc->products) {
      int d = 1, w = (int)p.weight();
      if (w) d--;
      while (w) { d++; w /= 10; }
      weightLeadDigit[1] = max(weightLeadDigit[1], d);
    }

    for (int j = -1; j < Ny; j++) {
      Array<TableCellInfo> col;
      if (j == -1) {
        for (int i = 0; i < Nx; i++) {
          col.push_back(TableCellInfo(this, columnName[i], font[1], 1));
        }
      } else {
        Product *pi = nullptr;
        if (j < (int)pDoc->products.size()) pi = &pDoc->products[j];

        SWPRINTF(noFmt, L"%2d", j + 1);
        col.push_back(TableCellInfo(this, noFmt, font[1], 1));

        if (pi) {
          auto unit = toWString(pi->unit.name);
          col.push_back(TableCellInfo(this, toWString(pi->name), font[1], 1));

          if (pi->quantity > 0) {
            {
              wchar_t tmp[32];
              int wI = (int)pi->weight();
              int wF = (int)((pi->weight() - wI) * 1000.0f);
              SWPRINTF(tmp, L"%%%dd.%%03d %%2s", weightLeadDigit[1]);
              SWPRINTF(noFmt, tmp, wI, wF, unit.c_str());
            }
            col.push_back(TableCellInfo(this, noFmt, font[1], 1));

            {
              wchar_t tmp[32];
              SWPRINTF(tmp, L"%%0%d" PRId64, *printer->batchLeadZero);
              SWPRINTF(noFmt, tmp, pi->batch.no);
            }
            col.push_back(TableCellInfo(this, noFmt, font[1], 1));
          } else {
            col.push_back(TableCellInfo(this, L"", font[1], 1));
            col.push_back(TableCellInfo(this, L"", font[1], 1));
          }
        } else {
          col.push_back(TableCellInfo(this, L"", font[0], 1));
          col.push_back(TableCellInfo(this, L"", font[1], 1));
          col.push_back(TableCellInfo(this, L"", font[1], 1));
        }

        col.push_back(TableCellInfo(this, L"България", font[1], 1));

        if (pi) {
          if (pi->quantity > 0) {
            auto date = toWString(pi->batch.exp.str(DateFormat::DMY, "."));
            col.push_back(TableCellInfo(this, date, font[1], 1));
          } else {
            col.push_back(TableCellInfo(this, L"", font[1], 1));
          }
        } else {
          col.push_back(TableCellInfo(this, L"", font[1], 1));
        }
      }
      tableStrings.push_back(std::move(col));
    }


    // calculate column width
    auto getMaxWidth = [&] (int col) {
      float w = -1.0f;
      for (auto &line : tableStrings) w = max(line[col].size.x + 100 * px.x, w);
      return w;
    };


    tableWidthMargins.resize(Nx + 1);
    tableWidthMargins[     0] = leftMargin;
    tableWidthMargins[Nx    ] = rightMargin;
    tableWidthMargins[Nx - 1] = tableWidthMargins[Nx - 0] - getMaxWidth(Nx - 1);
    tableWidthMargins[Nx - 2] = tableWidthMargins[Nx - 1] - getMaxWidth(Nx - 2);
    tableWidthMargins[Nx - 3] = tableWidthMargins[Nx - 2] - getMaxWidth(Nx - 3);
    tableWidthMargins[Nx - 4] = tableWidthMargins[Nx - 3] - getMaxWidth(Nx - 4);
    tableWidthMargins[     1] = tableWidthMargins[     0] + getMaxWidth(     0);

    {
      int j = 0;
      for (auto &row : tableStrings) {
        int i = 0;
        for (auto &str : row) {
          str.calcXPos(tableWidthMargins[i], tableWidthMargins[i + 1] - tableWidthMargins[i]);
          i++;
        }
        j++;
      }
    }

    // calculate row height
    auto getMaxHeight = [&] (int row) {
      float h = -1.0f;
      for (auto &cell : tableStrings[row]) h = max(cell.size.y + 50 * px.y, h);
      return h;
    };
    tableHeightMargins.push_back(toStr.pos.y + toStr.size.y * 1.5f);
    for (int j = 0; j < Ny; j++) {
      tableHeightMargins.push_back(tableHeightMargins.back() + getMaxHeight(j));
    }
    tableHeightMargins.push_back(tableHeightMargins.back() + getMaxHeight(Ny));

    {
      int j = 0;
      for (auto &row : tableStrings) {
        int i = 0;
        for (auto &str : row) {
          str.calcYPos(tableHeightMargins[j], tableHeightMargins[j + 1] - tableHeightMargins[j]);
          i++;
        }
        j++;
      }
    }




    ADD_EXTRA_STR(L"Посочените по-горе продукти отговарят на ветеринаро-"
                  L"санитарните и хигиенни изисквания на ЕС.", font[1], 1,
                  0.5f, tableHeightMargins.back() + 50 * px.y);

    ADD_EXTRA_STR(L"МПС № ................", font[1], 0,
                  leftMargin, noteStr.pos.y + noteStr.size.y * 5.0f);
    ADD_EXTRA_STR(L"Издал ................", font[1], 2,
                  rightMargin, noteStr.pos.y + noteStr.size.y * 5.0f);
  }


  vec2 calcText(const WString &txt, HFONT font) {
    RECT r = { 0, 0, 0, 0 };
    SelectObject(hDC, font);
    DrawTextW(hDC, txt.data(), (int)txt.length(), &r, DT_CALCRECT);
    return vec2(pixToUV(ivec2(abs(r.right - r.left), abs(r.bottom - r.top))));
  }

  void drawText(uint32_t color, const vec2 &uv, const vec2 &size, const WString &txt) {
    ivec2 p0 = uvToPix(uv);
    ivec2 p1 = uvToPix(uv + size);
    RECT r;
    r.left = p0.x; r.right = p1.x;
    r.top = p0.y; r.bottom = p1.y;
    DrawTextW(hDC, txt.data(), (int)txt.length(), &r, 0);
  }

  void drawText(uint32_t color, const vec2 &uv, const vec2 &size,
                const WString &txt, int from) {
    auto pos = uv;
    switch (from) {
      case 0: break;
      case 1: pos.x -= size.x * 0.5f; break;
      case 2: pos.x -= size.x; break;
      case 3: pos.y -= size.y; break;
      case 4: pos.y -= size.y; pos.x -= size.x * 0.5f; break;
      case 5: pos.y -= size.y; pos.x -= size.x; break;
      default: break;
    }
    drawText(color, pos, size, txt);
  }

  void drawLine(uint32_t color, const vec2 &uv0, const vec2 &uv1) {
    auto p0 = uvToPix(uv0);
    auto p1 = uvToPix(uv1);

    MoveToEx(hDC, p0.x, p0.y, NULL);
    LineTo(hDC, p1.x, p1.y);
  }
  void drawRect(uint32_t color, const vec2 &uv, const vec2 &size) {
    auto pMin = uvToPix(uv);
    auto pMax = uvToPix(uv + size);
    //SelectObject(hDC, hPen);
    SetDCPenColor(hDC, color);
    Rectangle(hDC, pMin.x, pMin.y, pMax.x, pMax.y);
  }
};

void PrintJobImpl::print() {
  Scope doneSG([&] { done = true; });

  printToFile = false;
  if (!hDC) {
    String devName = (char *)pProp->dmDeviceName;
    if (devName.find("PDF") != String::npos) {
      printToFile = true;
    }
    hDC = CreateDC(NULL, (LPCSTR)pProp->dmDeviceName, NULL, pProp);
    if (!hDC) {
      return;
    }
  }

  imgSize.x = GetDeviceCaps(hDC, HORZRES);
  imgSize.y = GetDeviceCaps(hDC, VERTRES);
  isLandscape = (imgSize.x > imgSize.y);
  px = 1.0f / vec2(imgSize);
  px.y *= (float)imgSize.x / imgSize.y;

  for (auto &[_, f] : printer->fontInfo.fonts) {
    font.push_back((HFONT)f);
  }
  for (auto &[_, f] : printer->fontInfo.fontsBold) {
    fontBold.push_back((HFONT)f);
  }


  auto orgFont = SelectObject(hDC, fontBold.back()); // largest bold font

  Scope sg([&] {
    if (hDC) {
      SelectObject(hDC, orgFont);
      DeleteDC(hDC);
    }
  });


  int docIdx = 0;
  if (isLandscape) {
    for (auto doc : pDoc) {
      if (!doc) continue;

      WString docName = toWString(u8"ТърговскиДокумент");
      wchar_t tmp[32], noFmt[32];
      SWPRINTF(tmp, L"%%0%d" PRId64, *printer->docNumLeadZero);
      SWPRINTF(noFmt, tmp, doc->no);
      docName += WString(L"_") + noFmt;

      if (startDoc(docName)) {
        if (StartPage(hDC) > 0) {
          renderDoc(doc, docIdx++);
          EndPage(hDC);
        }
        EndDoc(hDC);
      }
    }
  } else {
    WString docName = toWString(u8"ТърговскиДокумент");
    for (auto doc : pDoc) {
      if (!doc) continue;
      wchar_t tmp[32], noFmt[32];
      SWPRINTF(tmp, L"%%0%d" PRId64, *printer->docNumLeadZero);
      SWPRINTF(noFmt, tmp, doc->no);
      docName += WString(L"_") + noFmt;
    }
    if (startDoc(docName)) {
      if (StartPage(hDC) > 0) {
        for (auto doc : pDoc) renderDoc(doc, docIdx++);
        EndPage(hDC);
      }
      EndDoc(hDC);
    }
  }
}

bool PrintJobImpl::startDoc(const WString &docName) {
  WString docPath = L"PDF\\" + docName + L".pdf";
  DOCINFOW docInfo = { 0 };
  docInfo.cbSize = sizeof(DOCINFO);
  docInfo.fwType = 0;
  docInfo.lpszDocName = docName.c_str();
  if (printToFile) {
    createDirectories("PDF");
    docInfo.lpszOutput = docPath.c_str();
  }

  auto printJobID = StartDocW(hDC, &docInfo);
  if (printJobID < 0) {
    return false;
  }
  jobId = printJobID;
  return true;
}

void PrintJobImpl::renderDoc(TradeDoc *doc, int docIdx) {
  if (!doc) return;

  uint32_t black = RGB(0, 0, 0);

  vec4 margins; // top, left, right, bottom
  margins.y = 0.062f; // left
  margins.z = 1.0f - margins.y; // right
  if (isLandscape) {
    margins.x = 0.062f; // top
    margins.w = 1.0f - margins.x; // bottom
  } else {
    if (docIdx == 0) {
      margins.x = 0.062f; // top
      margins.w = 0.5f - margins.x; // bottom
    } else {
      margins.x = 0.5f + 0.062f; // top
      margins.w = 1.0f - margins.x; // bottom
    }
  }

  genTableStrings(doc, margins);

  for (auto &e : extraStrings) e.draw();
  for (auto &row : tableStrings) {
    for (auto &cell : row) cell.draw();
  }

  float top = tableHeightMargins[0];
  float bottom = tableHeightMargins.back();
  float left = tableWidthMargins[0];
  float right = tableWidthMargins.back();
  for (auto x : tableWidthMargins)  drawLine(black, vec2(x, top), vec2(x, bottom));
  for (auto y : tableHeightMargins) drawLine(black, vec2(left, y), vec2(right, y));
}

PrintJob IPrintJob::create(IPrinter *pr, Pair<TradeDoc *, TradeDoc *> docs, const DEVMODE *props) {
  if (!pr || !docs.first) return nullptr;
  auto j = MakeHandle(PrintJobImpl);
  if (j) {
    j->printer = pr;
    j->pDoc[0] = docs.first;
    j->pDoc[1] = docs.second;
    j->pProp = props;
  }
  return j;
}
PrintJob IPrintJob::create(IPrinter *pr, Pair<TradeDoc *, TradeDoc *> docs, void *hDC) {
  if (!pr || !docs.first) return nullptr;
  auto j = MakeHandle(PrintJobImpl);
  if (j) {
    j->printer = pr;
    j->pDoc[0] = docs.first;
    j->pDoc[1] = docs.second;
    j->hDC = (HDC)hDC;
  }
  return j;
}