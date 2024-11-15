#pragma once

#include "common.h"
#include "imgui_addons/ImTextField.h"
#include "imgui_addons/ImDatePicker.h"
#include "TradeDocumentView.h"
#include "BatchSettings.h"
#include "ProductSettings.h"

DefineHandle(IMainWindow, MainWindow);
class IMainWindow : public OnGLWindowEvent {
protected:
  bool running = true;

  WString name = L"Търговски документ";

  // product helpers
  Array<String> columnInfo;
  bool openSettings = false;
  BatchSettings batchSettings;
  ProductSettings productSettings;

  USet<TradeDoc *> selectedDocs;

  // new doc helpers
  bool isDocNumberEdited = false;
  int64_t docNumberEdit = 0;

  bool isDocDateEdited = false;
  ImDatePicker docDatePicker;

  ImTextField receiverEdit;

  Map<int, int> quantities;

  // database
  TradeDB db;

  // rendering
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
  void renderNewDoc();
  void renderDatabase();
  void renderSettings();

  Map<TradeDoc *, TradeDocView> docView;

public:
  IMainWindow(Array<uint8_t> &fontData);
  virtual ~IMainWindow();

  int run();
  bool isRunning() const { return running; }

  static MainWindow create(Array<uint8_t> &fontData);
};