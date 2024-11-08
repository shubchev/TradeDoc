#include "MainWindow.h"
#include <inttypes.h>

static const char *monthNames[] = {
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec",
};

IMainWindow::IMainWindow() {
  if (!GL::init()) {
    throw RuntimeError("Failed to load GL");
  }

  GL::setWindowTitle("Trade Document");
  GL::addListener(this);
  GL::clearColor(Color_Grey);
  GL::setWindowSize(1200, 600);
  GL::showWindow();

  int i = 10000;
  TradeDoc d;
  d.date = ImDatePicker::getToday();
  d.date.year--;
  d.no = i++;
  d.recipient = "Rec1";
  database.add(d);

  d.no = i++;
  d.recipient = "Rec1-1";
  database.add(d);
  d.no = i++;
  d.date.day++;
  d.recipient = "Rec1-2";
  database.add(d);

  d.date = ImDatePicker::getToday();
  d.no = i++;
  d.date.day++;
  d.recipient = "Rec2";
  database.add(d);

  d.date = ImDatePicker::getToday();
  d.date.year++;
  d.no = i++;
  d.date.month++;
  d.date.day--;
  d.recipient = "Rec3";
  database.add(d);

  auto glInitGUI = [&] () {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "imgui.ini";

    static Array<ImWchar> glyphs;
    auto cyrillic = io.Fonts->GetGlyphRangesCyrillic();
    for (int i = 0; cyrillic[i] != 0; i++) {
      glyphs.push_back(cyrillic[i]);
    }
    glyphs.push_back(0x2116); glyphs.push_back(0x2116);
    glyphs.push_back(0x0);

    // TODO: load from resource file
    io.Fonts->AddFontFromFileTTF("D:\\workspace\\SHGE\\resources\\fonts\\FreeMono.ttf", 20,
                                 nullptr, glyphs.data());

    //auto &style = ImGui::GetStyle();
    //style.Colors[ImGuiCol_WindowBg] = Color_Grey;

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)GL::getGLWindowHandle(), true);
    const char *glsl_version = "#version 400";
    ImGui_ImplOpenGL3_Init(glsl_version);
  };
  GL::exec(glInitGUI);

  docDate = ImDatePicker::getToday();
}

IMainWindow::~IMainWindow() {
}

MainWindow IMainWindow::create() {
  auto mw = MakeHandle(IMainWindow);
  if (mw) {
  }
  return mw;
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
      if (ImGui::MenuItem("Open")) {

      }
      ImGui::Separator();
      if (ImGui::MenuItem("Exit")) {
        GL::notifyWindowClose();
      }
      ImGui::EndMenu();
    }
  }
  ImGui::EndMainMenuBar();

  DockSpaceOverViewport();

  auto menuBarSize = ImGui::GetItemRectSize();

  if (ImGui::Begin("Database##dbViewWin")) {
    renderDatabase();
  }
  ImGui::End();


  if (ImGui::Begin("New##mainWin")) {
    ImGui::Text(u8"№"); ImGui::SameLine(150);
    if (isDocNumberEdited) {
      auto ret = docNumberEdit.render("##docNumEdit");
      if (ret) {
        docNumber = std::atoll(docNumberEdit.getText().c_str());
        isDocNumberEdited = false;
      } else if (!docNumberEdit.hasFocus() || ret) {
        isDocNumberEdited = false;
      }
    } else {
      char fmt[10];
      SPRINTF(fmt, "%%0%dd", docNumLeadZero);
      ImGui::Text(fmt, docNumber);
      if (ImGui::IsItemHovered()) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          isDocNumberEdited = true;
          docNumberEdit.setFormat(ImTextFieldFormat::PositiveInteger);
          docNumberEdit.focus();
          docNumberEdit.setText("%" PRId64, docNumber);
        } else {
          ImGui::BeginTooltip();
          ImGui::Text("Double click to change.\nPress ENTER to accept new number");
          ImGui::EndTooltip();
        }
      }
    }

    ImGui::Text(u8"Дата"); ImGui::SameLine(150);
    if (isDocDateEdited) {
      //ImGui::SetNextItemWidth(200);
      auto res = docDatePicker.render("##date");
      if (res == ImDatePickerResult::NewSel) {
        docDate = docDatePicker.getSelection();
        docDatePicker.close();
        isDocDateEdited = false;
      } else if (res == ImDatePickerResult::Cancel) {
        docDatePicker.close();
        isDocDateEdited = false;
      }
    } else {
      ImGui::Text("%02d %s %04d",
                  docDate.day, ImDatePicker::getShortMonthName(docDate), docDate.year);
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
    static char receiver[1024] = { 0 };
    ImGui::SetNextItemWidth(450);
    ImGui::InputText("##recv", receiver, sizeof(receiver));

    ImGui::Separator();


    class ColumnInfo {
    public:
      String name;
      int type = 0; // 0 label, 1 int, 2 float, 3 date
      int dataSource = -1; // 0 - auto inc, N - tableN, -1 editable
    };

    std::vector<ColumnInfo> columnInfo = {
      { u8"№", 0, 0 },
      { u8"Продукт", 0, 1 },
      { u8"Тегло", 2, -1 },
      { u8"№ на партида", 1, -1 },
      { u8"Годност", 3, -1 },
    };


    Map<int, Array<String>> dataSource = {
      { 0,
        {
          u8"Масло от Бял Трън   Ф. - 1.000 л.",
          u8"Масло от Бял Трън   Ф. - 0.500 л.",
          u8"Масло от Бял Трън   Ф. - 0.250 л.",
          u8"Масло от Бял Трън Н.Ф. - 0.750 л.",
          u8"Масло от Бял Трън Н.Ф. - 0.250 л.",

          u8"Брашно от Бял Трън     - 1.000 кг.",
          u8"Брашно от Бял Трън     - 0.500 кг.",
          u8"Брашно от Бял Трън     - 0.250 кг.",

          u8"Тахан от Бял Трън      - 0.300 кг.",
        }
      },
    };

    int Nx = 5;
    ImGuiTableFlags tbflags = ImGuiTableFlags_SizingFixedFit |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_Borders;
    if (ImGui::BeginTable("##newDocTable", Nx, tbflags)) {
      ImGui::TableSetupColumn("No", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("No1", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("No2", ImGuiTableColumnFlags_WidthFixed);

      int Ny = 12 + 1;
      for (int j = 0; j < Ny; j++) {
        ImGui::TableNextRow();
        for (int i = 0; i < Nx; i++) {
          ImGui::TableNextColumn();

          auto strId = toString(j) + "x" + toString(i);
          if (j > 0) {
            if (i == 0) {
              ImGui::Text("%d", j);
            } else if (i == 1) {
              if (j - 1 < (int)dataSource[0].size()) ImGui::Text(dataSource[0][j - 1].c_str());
            } else if (i == 2) {
              ImGui::SetNextItemWidth(100);
              docProdTextField[j - 1][i].setFormat(ImTextFieldFormat::PositiveNumber);
              docProdTextField[j - 1][i].render("##dataedit" + strId);
            } else if (i == 3) {
              ImGui::SetNextItemWidth(100);
              docProdTextField[j - 1][i].setFormat(ImTextFieldFormat::PositiveInteger);
              docProdTextField[j - 1][i].render("##dataedit" + strId);
            } else if (i == 4) {
              //ImGui::SetNextItemWidth(100);
              docProdValidity[j - 1][i].render("##dataedit" + strId);
            }
          } else {
            ImGui::TextColored(Color_Red, columnInfo[i].name.c_str());
          }

        }
      }
      ImGui::EndTable();
    }




  }
  ImGui::End();

}

void IMainWindow::renderDatabase() {
  for (auto &[year, mdb] : database.db) {
    if (!ImGui::TreeNodeEx(toString(year).c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) continue;

    for (auto &[month, ddb] : mdb) {
      auto dName = ImDatePicker::getShortMonthName(month);
      if (!ImGui::TreeNodeEx(dName, ImGuiTreeNodeFlags_OpenOnArrow)) continue;

      for (auto &[day, db] : ddb) {
        if (!ImGui::TreeNodeEx(toString(day).c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) continue;

        for (auto &[no, td] : db) {
          ImGui::Selectable(toString(no).c_str());
        }

        ImGui::TreePop(); // day
      }

      ImGui::TreePop(); // month
    }

    ImGui::TreePop(); // year
  }
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
