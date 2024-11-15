#include "TradeDocumentView.h"


TradeDocView::TradeDocView() {

}
TradeDocView::TradeDocView(TradeDB &_db, TradeDoc &doc,
                           uint32_t dNT, Array<String> *ci)
  : db(&_db), mDoc(doc), dockNextTo(dNT), columnInfo(ci) {
  char fmt[32] = { 0 };
  SPRINTF(fmt, "%%0%d" PRId64, *db->docNumLeadZero);
  SPRINTF(title, fmt, doc.no);
  openFlag = true;
}

void TradeDocView::focus() {
  focusFlag = true;
}

void TradeDocView::close() {
  openFlag = false;
}

bool TradeDocView::render() {
  if (!openFlag) {
    return false;
  }

  Scope winSG([] {
    ImGui::End();
  });

  if (focusFlag && !firstOpenRenderView) {
    ImGui::SetNextWindowFocus();
    focusFlag = false;
  }

  uint32_t flags = ImGuiWindowFlags_NoSavedSettings;
  if (!ImGui::Begin(title, &openFlag, flags)) {
    return false;
  }

  char fmt[32] = { 0 };
  SPRINTF(fmt, u8"№:         %%0%d" PRId64, *db->docNumLeadZero);
  ImGui::Text(fmt, mDoc.no);
  ImGui::Text(u8"Дата:      %s", mDoc.date.str(DateFormat::SNamedDMY).c_str());
  ImGui::Text(u8"Получател: %s", mDoc.recipient.c_str());

  int Nx = (int)columnInfo->size();
  ImGuiTableFlags tbflags = ImGuiTableFlags_SizingFixedFit |
    ImGuiTableFlags_RowBg |
    ImGuiTableFlags_Borders;
  if (ImGui::BeginTable("##viewDocTable", Nx, tbflags)) {
    for (int i = 0; i < Nx; i++) {
      char tmp[32];
      SPRINTF(tmp, "VTNo%d", i);
      if (i == 1) ImGui::TableSetupColumn(tmp, ImGuiTableColumnFlags_WidthStretch);
      else ImGui::TableSetupColumn(tmp, ImGuiTableColumnFlags_WidthFixed);
    }

    char editFmt[16];
    SPRINTF(editFmt, "%%0%dd", *db->batchLeadZero);

    int Ny = (int)mDoc.products.size();
    for (int j = -1; j < Ny; j++) {
      ImGui::TableNextRow();
      for (int i = 0; i < Nx; i++) {
        ImGui::TableNextColumn();

        if (j >= 0) {
          auto &pi = mDoc.products[j];
          if (i == 0) {
            ImGui::Text("%2d", j + 1);
          } else if (i == 1) {
            ImGui::Text(pi.name.c_str());
          } else if (i == 2) {
            ImGui::Text("%d", pi.quantity);
          } else if (i == 3) {
            ImGui::Text("%0.3f %s", pi.unit.coef, pi.unit.name.data());
          } else if (i == 4) {
            ImGui::Text("%0.3f", pi.weight());
          } else if (i == 5) {
            ImGui::Text(editFmt, pi.batch.no);
          } else if (i == 6) {
            auto ds = pi.batch.exp.str(DateFormat::SNamedDMY, " ");
            ImGui::Text(ds.c_str());
          }
        } else {
          ImGui::TextColored(Color_Red, (*columnInfo)[i].c_str());
        }

      }
    }
    ImGui::EndTable();
  }

  if (ImGui::Button((String("Print##") + title).c_str())) {
    db->printerSettings.print({ &mDoc }, false);
  }
  ImGui::SameLine();
  if (ImGui::Button((String("Print Setup##") + title).c_str())) {
    db->printerSettings.print({ &mDoc }, true);
  }


  if (firstOpenRenderView) {
    ImGuiDockNodeFlags noDockFlags = ImGuiDockNodeFlags_NoDockingSplitMe |
      ImGuiDockNodeFlags_NoDockingSplitOther |
      ImGuiDockNodeFlags_NoDockingOverOther |
      ImGuiDockNodeFlags_NoDockingOverEmpty;
    ImGuiDockNode *node;
#define DOCK_WIN(name, id) { \
        ImGui::DockBuilderDockWindow(name, id); \
        node = ImGui::DockBuilderGetNode(id); \
        node->LocalFlags |= noDockFlags; \
    }

    DOCK_WIN(title, dockNextTo);
    firstOpenRenderView = false;
  }

  return true;
}