#pragma once

#include "common.h"
#include "Label.h"
#include "TextBox.h"
#include "DatePicker.h"
#include "UpDown.h"
#include "DataGrid.h"
#include "TradeDocument.h"

DefineHandle(IMainWindow, MainWindow);
class IMainWindow : public IWinEventCb {
protected:
  bool running = true;

  WNDCLASS wcx;
  HWND hMainWin = NULL;
  HMENU hMainMenu = NULL;
  LPCWSTR className = TEXT("TradeDocWndClass");
  std::wstring name = TEXT("Търговски документ");

  HFONT hViewFont = NULL;

  bool isDocNumberEdited = false;
  bool docNumberQuestion = false;
  int64_t docNumberBeforeEdit = 0;
  Label docNumberLabel;
  Label receiverLabel;
  Label dateLabel;

  int docNumLeadZero = 9;
  UpDownPicker docNumberEdit;
  TextBox receiver;
  DatePicker datePick;

  DataGrid dataGrid;

  TradeDoc doc;

  std::vector<Label> docEditColumns;
  std::vector<std::vector<WinSubclass>> docEditGrid;


  std::vector<IWinSubclass *> children;

  bool OnWinEvent(IWinSubclass *win, UINT msg, WPARAM wParam, LPARAM lParam) override;

public:
  IMainWindow();
  virtual ~IMainWindow();



  static LRESULT CALLBACK WinMainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  int run();
  bool isRunning() const { return running; }
  LRESULT processEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  bool New();
  bool Save();
  bool Open();
  void Close();

  static MainWindow create();
};