#pragma once

#include "common.h"
#include "imgui_addons/ImTextField.h"
#include "imgui_addons/ImDatePicker.h"
#include "TradeDocument.h"

DefineHandle(IMainWindow, MainWindow);
class IMainWindow : public OnGLWindowEvent {
protected:
  bool running = true;

  WString name = L"Търговски документ";

  bool isDocNumberEdited = false;
  ImTextField docNumberEdit;
  int64_t docNumber = 0;
  int docNumLeadZero = 9;

  bool isDocDateEdited = false;
  CalendarTime docDate;
  ImDatePicker docDatePicker;

  String docReceiver;


  Map<int, Map<int, ImTextField>> docProdTextField;
  Map<int, Map<int, ImDatePicker>> docProdValidity;

  TradeDB database;
  TradeDoc doc;

  void DockSpaceOverViewport();
  uint32_t mainWinViewID = 0;
  uint32_t dbViewID = 0;

  void onResize(int width, int height) override;
  void onDropPath(const Array<Path> &paths) override;
  void onMouseMove(int xpos, int ypos) override;
  void onMouseMoveDelta(int dx, int dy) override;
  void onMousePress(MouseButton btn) override;
  void onMouseRelease(MouseButton btn) override;
  void onMouseScroll(int dx, int dy) override;
  void onKeyUp(KeyButton btn, const Array<KeyButton> &mods) override;
  void onKeyDown(KeyButton btn, const Array<KeyButton> &mods) override;

  void render();
  void renderDatabase();

public:
  IMainWindow();
  virtual ~IMainWindow();

  int run();
  bool isRunning() const { return running; }

  bool New();
  bool Save();
  bool Open();
  void Close();

  static MainWindow create();
};