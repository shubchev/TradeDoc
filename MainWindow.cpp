#include "MainWindow.h"
#include <CommCtrl.h>

HINSTANCE g_hInst;

IMainWindow::IMainWindow() {
  memset(&wcx, 0, sizeof(wcx));
  wcx.hInstance = g_hInst;
  wcx.lpszClassName = className;
  wcx.lpfnWndProc = IMainWindow::WinMainProc;
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW);

  if (!RegisterClass(&wcx)) {
    wcx.hInstance = NULL;
    throw std::runtime_error("Failed to register window class");
  }
  hMainMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MAINMENU));
  if (!hMainMenu) {
    throw std::runtime_error("Failed to load window menu");
  }

  hMainWin = CreateWindow(className, name.c_str(), WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL,
                          hMainMenu, g_hInst, NULL);
  if (!hMainWin) {
    throw std::runtime_error("Failed to create window");
  }

  SetWindowLongPtr(hMainWin, GWLP_USERDATA, (intptr_t)this);

  hViewFont = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
                           OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                           DEFAULT_PITCH | FF_DONTCARE, TEXT("Consolas"));

  SendMessage(hMainWin, WM_SETFONT, (WPARAM)hViewFont, TRUE);

  INITCOMMONCONTROLSEX iccx;
  iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
  iccx.dwICC = ICC_DATE_CLASSES | ICC_LISTVIEW_CLASSES | ICC_UPDOWN_CLASS | ICC_TAB_CLASSES;
  if (!InitCommonControlsEx(&iccx)) {
    throw std::runtime_error("Failed to initialize common controls");
  }

  int lineHeight = 20;

  try {
    int lineOffset = 10;
    int maxWidth = 0;

#define ADD_LABEL(which, text) \
    which = ILabel::create(hMainWin, 10, lineOffset, 100, lineHeight); \
    which->setText(TEXT(text)); \
    maxWidth = (maxWidth < which->getWidth()) ? which->getWidth() : maxWidth; \
    lineOffset += 30;

    ADD_LABEL(docNumberLabel, "№");
    ADD_LABEL(dateLabel, "Дата");
    ADD_LABEL(receiverLabel, "Получател");

    maxWidth += 20;
    lineOffset = 5;

    docNumberEdit = IUpDownPicker::create(hMainWin, maxWidth, lineOffset, 140, lineHeight);
    docNumberEdit->setY(docNumberLabel->getY() + docNumberLabel->getHeight() / 2 - docNumberEdit->getHeight() / 2);
    docNumberEdit->setText(TEXT("%%0%dd"), docNumLeadZero);
    docNumberEdit->setMin(0);
    docNumberEdit->setMax((int)pow(10, docNumLeadZero) - 1);
    docNumberEdit->setCurrentValue(4);
    docNumberEdit->hideArrows();
    docNumberEdit->setReadOnly();


    datePick = IDatePicker::create(hMainWin, maxWidth, lineOffset, 140, lineHeight);
    datePick->setY(dateLabel->getY() + dateLabel->getHeight() / 2 - datePick->getHeight() / 2);

    receiver = ITextBox::create(hMainWin, maxWidth, lineOffset, 140, lineHeight);
    receiver->setY(receiverLabel->getY() + receiverLabel->getHeight() / 2 - receiver->getHeight() / 2);
    receiver->setWidth(receiver->getWidth() * 3);


    class ColumnInfo {
    public:
      std::wstring name;
      int type = 0; // 0 label, 1 int, 2 float, 3 date
      int dataSource = -1; // 0 - auto inc, N - tableN, -1 editable
    };

    std::vector<ColumnInfo> columnInfo = {
      { TEXT("№"), 0, 0 },
      { TEXT("Продукт"), 0, 1 },
      { TEXT("Тегло"), 2, -1 },
      { TEXT("№ на партида"), 1, -1 },
      { TEXT("Годност"), 3, -1 },
    };

    std::map<int, std::vector<std::wstring>> dataSource = {
      { 0,
        {
          TEXT("Масло от Бял Трън   Ф.- 1.000 л."),
          TEXT("Масло от Бял Трън   Ф.- 0.500 л."),
          TEXT("Масло от Бял Трън   Ф.- 0.250 л."),
          TEXT("Масло от Бял Трън Н.Ф.- 0.750 л."),
          TEXT("Масло от Бял Трън Н.Ф.- 0.250 л."),

          TEXT("Брашно от Бял Трън - 1.000 кг."),
          TEXT("Брашно от Бял Трън - 0.500 кг."),
          TEXT("Брашно от Бял Трън - 0.250 кг."),

          TEXT("Тахан от Бял Трън"),
        }
      },
    };

    int maxProductNameLen = 0;
    for (auto &t : dataSource[0])
      maxProductNameLen = max(maxProductNameLen, ILabel::getTextLength(hMainWin, t));


    int Nx = (int)columnInfo.size();
    int Ny = 12;
    if (0) {
      std::vector<int> offsets;
      std::vector<int> columnSize;
      int offset = 10;
      lineOffset = receiver->getY() + receiver->getHeight() + lineHeight;
      for (int i = 0; i < Nx; i++) {
        if (i) {
          if (i == 1) offset += max(docEditColumns.back()->getWidth(), 30);
          else if (i == 2) offset += max(docEditColumns.back()->getWidth(), maxProductNameLen);
          else offset += max(docEditColumns.back()->getWidth(), 100);
          offset += 10;

          columnSize.push_back(offset - offsets.back() - 10);
        }
        offsets.push_back(offset);
        docEditColumns.push_back(ILabel::create(hMainWin, offset, lineOffset, 10, 10));
        docEditColumns.back()->setText(columnInfo[i].name);
      }
      offset += max(docEditColumns.back()->getWidth(), 100);
      columnSize.push_back(offset - offsets.back());

      for (int j = 0; j < Ny; j++) {
        docEditGrid.push_back({});
        auto &line = docEditGrid.back();
        lineOffset += 30;

        for (auto &ci : columnInfo) {
          int i = (int)line.size();
          switch (ci.type) {
            case 0: {
              if (ci.dataSource == 0) {
                auto c = ILabel::create(hMainWin, offsets[line.size()], lineOffset, columnSize[i], lineHeight);
                c->setFont(hViewFont);
                c->setText(std::to_wstring(j + 1));
                line.push_back(c);
              } else if (ci.dataSource > 0) {
                auto &ds = dataSource[ci.dataSource - 1];
                if (j < (int)ds.size()) {
                  auto c = ILabel::create(hMainWin, offsets[line.size()], lineOffset, columnSize[i], lineHeight);
                  c->setFont(hViewFont);
                  c->setText(ds[j]);
                  line.push_back(c);
                } else {
                  auto c = ITextBox::create(hMainWin, offsets[line.size()], lineOffset, columnSize[i], lineHeight);
                  c->setFont(hViewFont);
                  line.push_back(c);
                }
              }
              break;
            }
            case 1: {
              auto c = ITextBox::create(hMainWin, offsets[line.size()], lineOffset, columnSize[i], lineHeight);
              c->setFont(hViewFont);
              c->setTextFilter(TextFilter::PositiveInteger);
              line.push_back(c);
              break;
            }
            case 2: {
              auto c = ITextBox::create(hMainWin, offsets[line.size()], lineOffset, columnSize[i], lineHeight);
              c->setFont(hViewFont);
              c->setTextFilter(TextFilter::PositiveNumber);
              line.push_back(c);
              break;
            }
            case 3: {
              auto c = IDatePicker::create(hMainWin, offsets[line.size()], lineOffset, columnSize[i], lineHeight);
              c->setFont(hViewFont);
              line.push_back(c);
              break;
            }
            default: break;
          }
        }
      }


      RECT wr, cr;
      GetWindowRect(hMainWin, &wr);
      GetClientRect(hMainWin, &cr);
      int ex = abs(wr.right - wr.left) - abs(cr.right - cr.left);
      int ey = abs(wr.bottom - wr.top) - abs(cr.bottom - cr.top);
      int maxW = 0, maxH = 0;
      for (auto &ll : docEditGrid) for (auto &lc : ll) {
        if (maxW < lc->getX() + lc->getWidth())  maxW = lc->getX() + lc->getWidth();
        if (maxH < lc->getY() + lc->getHeight()) maxH = lc->getY() + lc->getHeight();
      }
      SetWindowPos(hMainWin, NULL, wr.left, wr.top, maxW + ex + 10, maxH + ey + 10, 0);
    }

    dataGrid = IDataGrid::create(hMainWin, 10, 200, 500, 300);
    dataGrid->setFont(hViewFont);
    dataGrid->addColumn(TEXT("col1"), 150);
    dataGrid->addColumn(TEXT("col2"), 150);
    dataGrid->addColumn(TEXT("col3"), 150);
    dataGrid->addItem();
    dataGrid->addItem();
    dataGrid->addItem();

    SetFocus(receiver->getHandle());

    children.push_back(docNumberEdit.get());
    children.push_back(receiver.get());
    children.push_back(datePick.get());
    children.push_back(dataGrid.get());
  } catch (std::exception &) {
    throw std::runtime_error("Failed to create GUI");
  }


  RECT r;
  GetClientRect(hMainWin, &r);

  /*if (!doc.init()) {
    throw std::runtime_error("Failed to create GUI");
  }*/

  ShowWindow(hMainWin, SW_SHOWNORMAL);
  UpdateWindow(hMainWin);
}

IMainWindow::~IMainWindow() {
  DeleteObject(hViewFont);
  if (hMainWin) DestroyWindow(hMainWin);
  if (wcx.hInstance) UnregisterClass(className, g_hInst);
}

MainWindow IMainWindow::create() {
  auto mw = MakeHandle(IMainWindow);
  if (mw) {
    mw->docNumberEdit->addEventCb(mw);
  }
  return mw;
}


int IMainWindow::run() {
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0) && running) {
    if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE) {
      if (isDocNumberEdited) {
        SetFocus(receiver->getHandle());
        continue;
      }
    } else if (msg.message == WM_KEYUP && msg.wParam == VK_RETURN) {
      if (isDocNumberEdited) {
        docNumberQuestion = true;
        auto cNum = docNumberEdit->getCurrentValue();
        DWORD selBeg = 0, selEnd = 0;
        SendMessage(docNumberEdit->getTextFieldHandle(), EM_GETSEL, (WPARAM)&selBeg, (LPARAM)&selEnd);
        auto ret = MessageBox(hMainWin, TEXT("Are you sure you want to change it?"),
                                        TEXT("Trade number change"),
                                        MB_ICONQUESTION | MB_YESNOCANCEL);
        switch (ret) {
          case IDYES:
            docNumberBeforeEdit = docNumberEdit->getCurrentValue();
            docNumberQuestion = false;
            SetFocus(docNumberEdit->getHandle());
            SetFocus(receiver->getHandle());
            break;
          case IDNO:
            docNumberEdit->setCurrentValue(docNumberBeforeEdit);
            docNumberQuestion = false;
            SetFocus(docNumberEdit->getHandle());
            SetFocus(receiver->getHandle());
            break;
          default:
            docNumberEdit->setCurrentValue(cNum);
            SetFocus(docNumberEdit->getTextFieldHandle());
            SendMessage(docNumberEdit->getTextFieldHandle(), EM_SETSEL, selBeg, selEnd);
            docNumberQuestion = false;
            break;
          
        }
        continue;
      }
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}


bool IMainWindow::New() {
  return true;
}
bool IMainWindow::Save() {
  return true;
}
bool IMainWindow::Open() {
  return true;
}
void IMainWindow::Close() {

}


bool IMainWindow::OnWinEvent(IWinSubclass *win, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (win == dynamic_cast<IWinSubclass *>(docNumberEdit.get())) {
    if (msg == WM_SETFOCUS && !docNumberQuestion) {
      docNumberBeforeEdit = docNumberEdit->getCurrentValue();
      docNumberEdit->setText(std::wstring(TEXT("%d")));
      docNumberEdit->showArrows();
      docNumberEdit->setEditable();
      isDocNumberEdited = true;
    }
    if (msg == WM_KILLFOCUS && !docNumberQuestion) {
      docNumberEdit->setText(TEXT("%%0%dd"), docNumLeadZero);
      docNumberEdit->setCurrentValue(docNumberBeforeEdit);
      docNumberEdit->hideArrows();
      docNumberEdit->setReadOnly();
      isDocNumberEdited = false;
    }
  }
  return false;
}


LRESULT CALLBACK IMainWindow::WinMainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  auto pThis = (IMainWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  return pThis->processEvent(hWnd, uMsg, wParam, lParam);
}

LRESULT IMainWindow::processEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_SETFOCUS: {
      if (hWnd == docNumberEdit->getTextFieldHandle()) {
        DebugBreak();
      }
      break;
    }
    case WM_KILLFOCUS: {
      break;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
      break;
    }
    case WM_NOTIFY: {
      for (auto &c : children) {
        if (c->getHandle() == ((LPNMHDR)lParam)->hwndFrom) {
          c->OnNotify(c->getHandle(), uMsg, wParam, lParam);
        }
      }
      break;
    }
    case WM_ERASEBKGND: {
      auto hDC = (HDC)wParam;
      if (hWnd == hMainWin) {
        RECT rc; GetClientRect(hWnd, &rc);
        HBRUSH brush = CreateSolidBrush(RGB(127, 127, 127));
        FillRect(hDC, &rc, brush);
        DeleteObject(brush);
        return 1;
      }
      break;
    }
    case WM_CTLCOLORSTATIC: {
      HDC hdcStatic = (HDC)wParam;
      COLORREF bkColor;
      if (lParam != (LPARAM)hWnd) {
        GetBkColor(hdcStatic);
        bkColor = GetBkColor(hdcStatic);
        SetBkColor(hdcStatic, RGB(127, 127, 127));
        return (LRESULT)((HBRUSH)(COLOR_BACKGROUND));
      }
      break;
    }
    case WM_SIZE: {

      RECT r;
      GetClientRect(hWnd, &r);
      int width = abs(r.left - r.right);
      int height = abs(r.bottom - r.top);
      //dataGrid->setSize(max(width - 20, 20), max(height - dataGrid->getY() - 10, 50));
      break;
    }
    case WM_COMMAND: {
      switch (LOWORD(wParam)) {
        case ID_FILE_EXIT: {
          Close();
          running = false;
          break;
        }
        default: break;
      }
      break;
    }

    case WM_CLOSE: {
      running = false;
      break;
    }
    default: return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
  return 0;
}