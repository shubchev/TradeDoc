#include "MainWindow.h"
#include "Themes.h"

void *IMainWindow::iniReadOpenFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, const char *name) {
  auto pThis = (IMainWindow *)handler->UserData;

  return (void *)1; // just to allow call of ReadLineFn
}

void IMainWindow::iniReadLineFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry, const char *line) {
  auto pThis = (IMainWindow *)handler->UserData;
  if (strstr(line, "ThemeIndex")) {
    int index = 0;
    if (SSCANF(line, "ThemeIndex = %d", &index) == 1) {
      setTheme(&pThis->themeIdx, index);
    }
  }
};
void IMainWindow::iniWriteAllFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *out_buf) {
  auto pThis = (IMainWindow *)handler->UserData;
  out_buf->appendf("[Theme][Data]\n");
  out_buf->appendf("ThemeIndex = %d", pThis->themeIdx);
};


IMainWindow::IMainWindow(Array<uint8_t> &fontData)
  : batchSettings(db), productSettings(db) {

  if (!GL::init()) {
    throw RuntimeError("Failed to load GL");
  }

  GL::setWindowTitle("Trade Document");
  GL::addListener(this);
  GL::clearColor(Color_Grey);
  GL::setWindowSize(1200, 600);
  GL::maximizeWindow();
  GL::showWindow();

  columnInfo = {
    u8"№",
    u8"Продукт",
    u8"Бройка",
    u8"1 бр.",
    u8"Тегло/Обем",
    u8"№ на партида",
    u8"Годност",
  };

  db.printerSettings.batchLeadZero = db.batchLeadZero;
  db.printerSettings.docNumLeadZero = db.docNumLeadZero;
  db.printerSettings.init();

  if (!db.open("db.fzip")) {
    throw RuntimeError(db.lastError);
  }

  if (!db.loadAll()) {
    throw RuntimeError(db.lastError);
  }

  db.docNumber = db.getMaxDocNo() + 1;


  auto glInitGUI = [&] () {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    initThemes();

    // Add .ini handle for UserData type
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "Theme";
    ini_handler.TypeHash = ImHashStr("Theme");
    ini_handler.ReadOpenFn = iniReadOpenFn;
    ini_handler.ReadLineFn = iniReadLineFn;
    ini_handler.WriteAllFn = iniWriteAllFn;
    ini_handler.UserData = this;
    ImGui::AddSettingsHandler(&ini_handler);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "settings.ini";


    static Array<ImWchar> glyphs;
    auto cyrillic = io.Fonts->GetGlyphRangesCyrillic();
    for (int i = 0; cyrillic[i] != 0; i++) {
      glyphs.push_back(cyrillic[i]);
    }
    glyphs.push_back(0x2116); glyphs.push_back(0x2116);
    glyphs.push_back(0x0);

    io.Fonts->AddFontFromMemoryTTF(fontData.data(), (int)fontData.size(), 20,
                                   nullptr, glyphs.data());

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)GL::getGLWindowHandle(), true);
    const char *glsl_version = "#version 400";
    ImGui_ImplOpenGL3_Init(glsl_version);
  };
  GL::exec(glInitGUI);

  db.docDate = CalendarDate::today();
}

IMainWindow::~IMainWindow() {
  db.saveInfo();
  db.close();
}

MainWindow IMainWindow::create(Array<uint8_t> &fontData) {
  return MakeHandle(IMainWindow, fontData);
}


void IMainWindow::onResize(int width, int height) {
  GL::viewport(0, 0, width, height);
}
void IMainWindow::onDropPath(const Array<Path> &paths) {

}
void IMainWindow::onMouseMove(int xpos, int ypos) {

}
void IMainWindow::onMouseMoveDelta(int dx, int dy) {

}
void IMainWindow::onMousePress(MouseButton btn) {

}
void IMainWindow::onMouseRelease(MouseButton btn) {

}
void IMainWindow::onMouseScroll(int dx, int dy) {

}
void IMainWindow::onKeyUp(KeyButton btn, const Array<KeyButton> &mods) {

}
void IMainWindow::onKeyDown(KeyButton btn, const Array<KeyButton> &mods) {

}

int IMainWindow::run() {

  GL::showWindow();
  while (!GL::isWindowClosed()) {
    auto glCall = [&] {
      GL::clearColorDepthBuffer();
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      render();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      GL::swapBuffers();
    };
    GL::exec(glCall);
  }
  return 0;
}



void IMainWindow::DockSpaceOverViewport() {
  ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar |
    ImGuiWindowFlags_NoDocking |
    ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus;

  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our
  // background and handle the pass-thru hole, so we ask Begin() to not render a background.
  if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
    windowFlags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("##DockSpace", nullptr, windowFlags);
  ImGui::PopStyleVar();
  ImGui::PopStyleVar(2);

  ImGuiID dockspaceID = ImGui::GetID("DockSpaceViewport");
  ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

  static bool firstTime = true;
  if (firstTime) {
    ImGui::DockBuilderRemoveNode(dockspaceID); // clear any previous layout
    ImGui::DockBuilderAddNode(dockspaceID, dockspaceFlags | ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoDockingSplitMe);
    ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

    dbViewID = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.2f, nullptr, &mainWinViewID);

    ImGuiDockNodeFlags noDockFlags = ImGuiDockNodeFlags_NoDockingSplitMe |
      ImGuiDockNodeFlags_NoDockingSplitOther |
      ImGuiDockNodeFlags_NoDockingOverMe |
      ImGuiDockNodeFlags_NoDockingOverOther |
      ImGuiDockNodeFlags_NoDockingOverEmpty;

    ImGuiDockNode *node;
#define DOCK_WIN(name, id) { \
    ImGui::DockBuilderDockWindow(name, id); \
    node = ImGui::DockBuilderGetNode(id); \
    node->LocalFlags |= noDockFlags; \
}

    noDockFlags ^= ImGuiDockNodeFlags_NoDockingOverMe;
    DOCK_WIN("New##mainWin", mainWinViewID);
    DOCK_WIN("Database##dbViewWin", dbViewID);

    //DOCK_WIN("Main View", renderViewDockID);
    //DOCK_WIN("Material Viewer", renderViewDockID);

    ImGui::DockBuilderFinish(dockspaceID);
  }

  ImGui::End();


  if (firstTime) {

  }


  firstTime = false;
}





void IMainWindow::render() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Products")) {
        openSettings = true;
        productSettings.open();
        batchSettings.open();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Exit")) {
        GL::notifyWindowClose();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Settings")) {
      if (ImGui::MenuItem("Print settings ...")) {
        db.printerSettings.openSettings();
      }
      ImGui::Separator();

      if (ImGui::BeginMenu("Leading zero")) {
        int org = *db.docNumLeadZero;
        if (ImGui::InputInt("Document number##LeadZero", db.docNumLeadZero.get(), 1, 1)) {
          *db.docNumLeadZero = clamp(*db.docNumLeadZero, 0, 9);
          if (*db.docNumLeadZero != org && !db.saveInfo()) {
            *db.docNumLeadZero = org;
          }
        }
        org = *db.batchLeadZero;
        if (ImGui::InputInt("Batch number##LeadZero", db.batchLeadZero.get(), 1, 1)) {
          *db.batchLeadZero = clamp(*db.batchLeadZero, 0, 9);
          if (*db.batchLeadZero != org && !db.saveInfo()) {
            *db.batchLeadZero = org;
          }
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Theme")) {
        int index = -1;
        if (ImGui::MenuItem("Classic")) {
          index = 0;
        }
        if (ImGui::MenuItem("Dark")) {
          index = 1;
        }
        if (ImGui::MenuItem("Light")) {
          index = 2;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Windows")) {
          index = 3;
        }
        if (ImGui::MenuItem("Pink")) {
          index = 4;
        }
        ImGui::EndMenu();
        if (setTheme(&themeIdx, index)) {
          ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
      }


      ImGui::EndMenu();
    }
  }
  ImGui::EndMainMenuBar();

  DockSpaceOverViewport();

  renderNewDoc();
  renderDatabase();
  renderSettings();
  db.printerSettings.renderSettings();
  for (auto it = docView.begin(); it != docView.end();) {
    if (!it->second.isOpen()) {
      it = docView.erase(it);
    } else {
      it->second.render();
      it++;
    }
  }

  static bool focusNewWin = false;
  if (!focusNewWin) {
    ImGui::FocusWindow(ImGui::FindWindowByName("New##mainWin"));
    receiverEdit.focus();
    focusNewWin = true;
  }
}

void renderDateEntry(const char *label,
                     const CalendarDate &date,
                     const CalendarDate &today,
                     const Color &cTextColor,
                     ImDatePicker *picker) {
  ImGui::SetNextItemWidth(100);
  auto dDays = date.daysSince(today);
  if (picker) {
    if (dDays <= 7) {
      if (dDays < 0) picker->setButtonTextColor(Color_Red);
      else picker->setButtonTextColor(Color_Yellow);
    } else picker->setButtonTextColor(cTextColor);
    picker->render(label);
  } else {
    auto ds = date.str(DateFormat::SNamedDMY);
    if (dDays <= 7) {
      if (dDays < 0) ImGui::TextColored(Color_Red, ds.c_str());
      else ImGui::TextColored(Color_Yellow, ds.c_str());
    } else ImGui::Text(ds.c_str());
  }
  if (dDays <= 7) {
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      if (dDays < 0) ImGui::Text(u8"Изтекъл срок");
      else ImGui::Text(u8"Соркът изтича след < 7 дни");
      ImGui::EndTooltip();
    }
  }
}

void IMainWindow::renderNewDoc() {
  Scope winSG([] { ImGui::End(); });
  if (!ImGui::Begin("New##mainWin")) {
    return;
  }

  ImGui::Text(u8"№"); ImGui::SameLine(150);
  if (isDocNumberEdited) {
    int64_t step = 1, newV = docNumberEdit;
    char fmt[10];
    SPRINTF(fmt, "%%0%dd", *db.docNumLeadZero);
    ImGui::InputScalar("##docNumEdit", ImGuiDataType_S64, &newV,
                       &step, &step, fmt);
    if (newV != docNumberEdit) {
      auto dt = (newV - docNumberEdit > 0) ? 1 : -1;
      bool found = false;
      while (1) {
        if (newV < 0) break;
        if (!db.hasDocNo(newV)) { found = true; break; }
        newV += dt;
      }
      if (found) docNumberEdit = newV;
    }
    docNumberEdit = max(docNumberEdit, 0ll);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      if (docNumberEdit >= 0 && !db.hasDocNo(docNumberEdit)) {
        db.docNumber = docNumberEdit;
      }
      isDocNumberEdited = false;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      isDocNumberEdited = false;
    }
  } else {
    char fmt[10];
    SPRINTF(fmt, "%%0%dd", *db.docNumLeadZero);
    ImGui::Text(fmt, db.docNumber);
    if (ImGui::IsItemHovered()) {
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        isDocNumberEdited = true;
        docNumberEdit = db.docNumber;
      } else {
        ImGui::BeginTooltip();
        ImGui::Text("Double click to change.");
        ImGui::EndTooltip();
      }
    }
  }

  ImGui::Text(u8"Дата"); ImGui::SameLine(150);
  if (isDocDateEdited) {
    //ImGui::SetNextItemWidth(200);
    auto res = docDatePicker.render("dateToday");
    if (res == ImDatePickerResult::NewSel) {
      db.docDate = docDatePicker.getSelection();
      isDocDateEdited = false;
    } else if (res == ImDatePickerResult::Cancel) {
      isDocDateEdited = false;
    }
  } else {
    ImGui::Text(db.docDate.str(DateFormat::SNamedDMY).c_str());
    if (ImGui::IsItemHovered()) {
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        docDatePicker.open();
        isDocDateEdited = true;
      } else {
        ImGui::BeginTooltip();
        ImGui::Text("Double click to change");
        ImGui::EndTooltip();
      }
    }
  }



  ImGui::Text(u8"Получател"); ImGui::SameLine(150);
  ImGui::SetNextItemWidth(450);
  receiverEdit.render("##recv");

  ImGui::Separator();


  auto today = CalendarDate::today();
  bool allowSave = true;

  int Nx = (int)columnInfo.size();
  ImGuiTableFlags tbflags = ImGuiTableFlags_SizingFixedFit |
                            ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_Borders;
  if (ImGui::BeginTable("##newDocTable", Nx, tbflags)) {
    for (int i = 0; i < Nx; i++) {
      char tmp[32];
      SPRINTF(tmp, "DTNo%d", i);
      if (i == 1) ImGui::TableSetupColumn(tmp, ImGuiTableColumnFlags_WidthStretch);
      else ImGui::TableSetupColumn(tmp, ImGuiTableColumnFlags_WidthFixed);
    }

    char editFmt[16];
    SPRINTF(editFmt, "%%0%dd", *db.batchLeadZero);

    for (int j = -1; j < (int)db.productTemplate.size(); j++) {
      ImGui::TableNextRow();
      for (int i = 0; i < Nx; i++) {
        ImGui::TableNextColumn();

        auto strId = toString(j) + "x" + toString(i);
        if (j >= 0) {
          auto &pi = db.productTemplate[j];
          if (i == 0) {
            ImGui::Text("%2d", j + 1);
          } else if (i == 1) {
            ImGui::Text(pi->name.c_str());
          } else if (i == 2) {
            ImGui::SetNextItemWidth(150);
            if (ImGui::InputInt(("##dataEdit" + strId).c_str(), &quantities[j], 1, 10)) {
              quantities[j] = max(quantities[j], 0);
            }
          } else if (i == 3) {
            ImGui::Text("%0.3f %2s", pi->unit.coef, pi->unit.name.data());
          } else if (i == 4) {
            ImGui::Text("%0.3f", pi->weight(quantities[j]));
          } else if (i == 5) {
            if (auto b = pi->batchEdit.lock()) ImGui::Text(editFmt, b->no);
            else {
              ImGui::Text("-----");
              allowSave = false;
            }
          } else if (i == 6) {
            if (auto b = pi->batchEdit.lock()) {
              renderDateEntry(nullptr, b->exp, today, Color_White, nullptr);
            } else {
              ImGui::Text("-----");
            }
          }
        } else {
          ImGui::TextColored(Color_Red, columnInfo[i].c_str());
        }

      }
    }
    ImGui::EndTable();
  }

  bool atLeastOneFilled = false;
  for (auto &[j, q] : quantities) if (q > 0) atLeastOneFilled = true;

  Set<ProductTemplate> noBatchSet;
  for (auto &p : db.productTemplate) {
    if (p->batchEdit.expired()) noBatchSet.insert(p);
  }

  allowSave = allowSave && atLeastOneFilled;
  allowSave = allowSave && (receiverEdit.getText().length() > 0);
  allowSave = allowSave && noBatchSet.empty();

  auto &style = ImGui::GetStyle();
  if (!allowSave) {
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_Button]);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_Button]);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
  }
  bool saveDoc = ImGui::Button("Save##Doc");
  if (!allowSave) {
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
  }
  if (ImGui::IsItemHovered()) {
    if (!allowSave) {
      String err;
      if (receiverEdit.getText().empty()) err += u8"Липсва получател\n";
      if (!atLeastOneFilled) err += u8"Няма въведено количество\n";
      for (auto &p : noBatchSet) {
        err += u8"Продукт " + p->name + u8" няма партида\n";
      }
      ImTooltip(Color_Red, err.c_str());
    }
  }

  if (allowSave && saveDoc) {
    TradeDoc td;

    for (size_t i = 0; i < db.productTemplate.size(); i++) {
      auto &pi = db.productTemplate[i];

      auto b = pi->batchEdit.lock();
      Product tp;
      tp.name = pi->name;
      tp.batch = pi->batch;
      tp.unit = pi->unit;
      tp.quantity = max(quantities[i], 0);

      td.products.push_back(std::move(tp));
    }
    td.recipient = receiverEdit.getText();
    td.no = db.docNumber;
    td.date = db.docDate;

    if (db.saveDoc(td)) {
      db.add(td);
      db.docNumber++;
      db.docDate = today;
      receiverEdit.clearText();
      quantities.clear();
    }
  }
}

template<class T> using RMap = Map<int, T, std::greater<int>>;
void IMainWindow::renderDatabase() {
  Scope winSG([] { ImGui::End(); });
  if (!ImGui::Begin("Database##dbViewWin")) {
    return;
  }

  ImGui::Text(u8"Филтър");
  ImGui::Indent();

  static bool filterByDate = false;
  ImGui::Checkbox(u8"По дата", &filterByDate);

  static ImDatePicker startDate, endDate;
  if (filterByDate) {
    ImGui::Text(u8"От: "); ImGui::SameLine();
    startDate.render("filterBegDate");

    ImGui::Text(u8"До: "); ImGui::SameLine();
    endDate.render("filterEndDate");
  }

  static char dbFilter[512];
  ImGui::Text(u8"Получател: "); ImGui::SameLine();
  ImGui::InputText("##dbFilter", dbFilter, sizeof(dbFilter));
  size_t filterLen = strlen(dbFilter);

  ImGui::Unindent();
  ImGui::Separator();

  RMap<RMap<RMap<Array<TradeDoc *>>>> dates;
  for (auto &[date, docs] : db.docs) {
    if (filterByDate) {
      if (date < startDate.getSelection() || date > endDate.getSelection()) {
        continue;
      }
    }
    for (auto &td : docs) {
      if (filterLen > 0) {
        if (td.recipient.find(dbFilter) == String::npos) continue;
      }

      dates[date.year()][date.month()][date.day()].push_back(&td);
    }
  }

  char noFmt[32];
  SPRINTF(noFmt, u8"   №: %%0%d" PRId64, *db.docNumLeadZero);

  uint32_t treeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
  static bool firstLoad = true;
  if (firstLoad) {
    treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
    //firstLoad = false;
  }

  Map<int, TradeDoc *> docIndex;
  int docIdx = 0;
  for (auto &[year, monthDays] : dates) {
    if (!ImGui::TreeNodeEx(toString(year).c_str(), treeFlags)) continue;

    for (auto &[month, days] : monthDays) {
      auto dName = ImDatePicker::getShortMonthName(month);
      if (!ImGui::TreeNodeEx(dName, treeFlags)) continue;

      for (auto &[day, docs] : days) {
        if (!ImGui::TreeNodeEx(toString(day).c_str(), treeFlags)) continue;

        for (auto &td : docs) {
          docIndex[docIdx++] = td;

          bool isSel = selectedDocs.count(td);
          ImGui::Selectable(td->recipient.c_str(), &isSel);
          if (ImGui::IsItemHovered()) {
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
              if (!docView.count(td)) {
                docView[td] = TradeDocView(db, *td, mainWinViewID, &columnInfo);
              }
              docView[td].focus();
            }
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
              bool shft = ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
                          ImGui::IsKeyDown(ImGuiKey_RightShift);
              bool ctrl = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                          ImGui::IsKeyDown(ImGuiKey_RightCtrl);
              if (shft) {
                if (selectedDocStart < 0) {
                  selectedDocStart = docIdx - 1;
                } else if (selectedDocEnd < 0) {
                  selectedDocEnd = docIdx - 1;
                }
              } else {
                selectedDocStart = docIdx - 1;
                selectedDocEnd = -1;
              }
              if (ctrl) {
                if (selectedDocs.count(td)) selectedDocs.erase(td);
                else selectedDocs.insert(td);
              } else {
                selectedDocs.clear();
                selectedDocs.insert(td);
              }
            }

            ImGui::BeginTooltip();
              ImGui::Text(u8"Дата: %s", td->date.str(DateFormat::SNamedDMY).c_str());
              char tmp[128]; SPRINTF(tmp, noFmt, td->no);
              ImGui::Text(tmp);
            ImGui::EndTooltip();
          }
        }

        ImGui::TreePop(); // day
      }
      ImGui::TreePop(); // month
    }
    ImGui::TreePop(); // year
  }

  if (selectedDocStart >= 0 && selectedDocEnd >= 0) {
    auto Imin = min(selectedDocStart, selectedDocEnd);
    auto Imax = max(selectedDocStart, selectedDocEnd);
    for (int i = Imin; i <= Imax; i++) {
      selectedDocs.insert(docIndex[i]);
    }
  }

  if (ImGui::IsWindowFocused() && selectedDocs.size()) {
    if ((ImGui::IsKeyPressed(ImGuiKey_Enter) ||
         ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))) {
      TradeDocView *tdv = nullptr;
      for (auto td : selectedDocs) {
        if (docView.count(td)) continue;
        docView[td] = TradeDocView(db, *td, mainWinViewID, &columnInfo);
        tdv = &docView[td];
      }
      if (tdv) tdv->focus();
    }
  }

  if (ImGui::BeginPopupContextWindow("##dbCtxMenu")) {
    ImGui::PushStyleColor(ImGuiCol_Button, vec4(0));
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, vec2(0, 0));
    vec2 btnSz = vec2(150, 0);
    if (selectedDocs.empty()) {
      if (ImGui::Button("Select all##dbCtxMenu", btnSz)) {
        for (auto &[year, monthDays] : dates) {
          for (auto &[month, days] : monthDays) {
            for (auto &[day, docs] : days) {
              for (auto &td : docs) selectedDocs.insert(td);
        }}}}
    } else {
      if (ImGui::Button("Deselect##dbCtxMenu", btnSz)) {
        selectedDocs.clear();
      }

      ImGui::Separator();
      if (ImGui::Button("Open##dbCtxMenu", btnSz)) {
        TradeDocView *tdv = nullptr;
        for (auto td : selectedDocs) {
          if (docView.count(td)) continue;
          docView[td] = TradeDocView(db, *td, mainWinViewID, &columnInfo);
          tdv = &docView[td];
        }
        if (tdv) tdv->focus();
      }
      ImGui::Separator();
      if (ImGui::Button("Print##dbCtxMenu", btnSz)) {
        Array<TradeDoc *> docs;
        for (auto &d : selectedDocs) docs.push_back(d);
        db.printerSettings.print(docs, false);
      }
      if (ImGui::Button("Print Setup##dbCtxMenu", btnSz)) {
        Array<TradeDoc *> docs;
        for (auto &d : selectedDocs) docs.push_back(d);
        db.printerSettings.print(docs, true);
      }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::EndPopup();
  }
}

void IMainWindow::renderSettings() {
  if (!openSettings) {
    productSettings.close();
    batchSettings.close();
    return;
  }

  uint32_t winFlags = ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_NoDocking;
  if (ImGui::Begin("Product batches", &openSettings, winFlags)) {
    if (ImGui::BeginTabBar("##settingsTabBar")) {
      if (ImGui::BeginTabItem(u8"Партиди##settings")) {
        batchSettings.render();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(u8"Продукти##settings")) {
        productSettings.render();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}
