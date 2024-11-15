#include "ProductSettings.h"

ProductSettings::TableColumn::TableColumn(const String &n, int f, int idx) {
  name = n;
  id = "ProductTableColumn" + toString(idx);
  flags = f;
}

void ProductSettings::TableColumn::setup() {
  ImGui::TableSetupColumn(id.c_str(), flags);
}




ProductSettings::ProductSettings(TradeDB &_db) : db(_db) {
  int idx = 0;
  columnInfo.push_back(TableColumn(u8"", ImGuiTableColumnFlags_WidthFixed, idx++));
  columnInfo.push_back(TableColumn(u8"№", ImGuiTableColumnFlags_WidthFixed, idx++));
  columnInfo.push_back(TableColumn(u8"Продукт", ImGuiTableColumnFlags_WidthStretch, idx++));
  columnInfo.push_back(TableColumn(u8"Единица", ImGuiTableColumnFlags_WidthFixed, idx++));
  columnInfo.push_back(TableColumn(u8"Тегло/Обем", ImGuiTableColumnFlags_WidthFixed, idx++));
  columnInfo.push_back(TableColumn(u8"№ на партида", ImGuiTableColumnFlags_WidthFixed, idx++));
}

void ProductSettings::open() {
  countAtOpen = (int)db.productTemplate.size();
  for (auto &p : db.productTemplate) p->setupHelpers(db);
}

void ProductSettings::close() {
  newProd.clear();
}

void ProductSettings::renderTableHeader() {
  for (auto &ci : columnInfo) {
    ImGui::TableNextColumn();
    ImGui::TextColored(Color_Red, ci.name.c_str());
  }
}

void ProductSettings::renderTableNewRow() {
  ImGui::TableNextColumn();

  ImGui::PushStyleColor(ImGuiCol_Button, vec4(0));
  if (ImGui::SmallButton("+##product")) {
    auto pi = IProductTemplate::create();
    if (pi) {
      pi->isNew = true;
      pi->setupHelpers(db);
      newProd.push_back(pi);
    }
  }
  ImGui::PopStyleColor();
}

void ProductSettings::renderTableRow(int row, int offset) {
  ProductTemplate pi;
  if (offset >= 0) pi = newProd[row];
  else pi = db.productTemplate[row];
  auto rowStr = toString(row + max(offset, 0));

  {
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Button, vec4(0));
    if (ImGui::SmallButton((String("x##prod") + rowStr).c_str())) {
      deleteIdx = row;
      deleteOff = (offset >= 0) ? 1 : 0;
    }
    ImGui::PopStyleColor();
  }

  {
    ImGui::TableNextColumn();
    if (offset < 0) ImGui::Text("%2d", row + 1);
    else ImGui::Text(" *");
  }

  {
    ImGui::TableNextColumn();
    if (pi->isNew) pi->nameEdit.render("##prodNameEdit" + rowStr);
    else ImGui::Text(pi->name.c_str());
  }

  {
    String item;
    for (auto &u : unitNames) {
      auto len = strlen(u);
      item.insert(item.end(), u, u + len);
      item.push_back(0);
    }
    item.push_back(0);

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(100);
    ImGui::Combo(("##unitNameEdit" + rowStr).c_str(), &pi->unitNameEdit, &item[0]);
  }


  {
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(100);
    ImGui::InputFloat(("##unitCoefEdit" + rowStr).c_str(), &pi->unitCoefEdit,
                      0.0f, 0.0f, "%0.4f", ImGuiInputTextFlags_CharsNoBlank);
  }

  {
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(150);

    int idx = 0, i = 1;
    for (auto &bi : db.batchTemplate) {
      if (auto b = pi->batchEdit.lock()) {
        if (b.get() == bi.get()) { idx = i; break; }
      }
      i++;
    }

    String item = "(N/A)"; item.push_back(0);
    for (auto &bi : db.batchTemplate) {
      char tmp[32]; SPRINTF(tmp, noFmt, bi->noEdit);
      item += tmp; item.push_back(0);
    }
    item.push_back(0);

    if (ImGui::Combo(("##prodBatchEdit" + rowStr).c_str(), &idx, item.c_str())) {
      if (idx >= 1 && idx - 1 < (int)db.batchTemplate.size()) {
        pi->batchEdit = db.batchTemplate[idx - 1];
      } else pi->batchEdit.reset();
    }
  }
}


void ProductSettings::render() {
  auto &style = ImGui::GetStyle();
  SPRINTF(noFmt, "%%0%d" PRId64, *db.batchLeadZero);

  ImGuiTableFlags tbflags = ImGuiTableFlags_SizingFixedFit |
                            ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_Borders;
  if (ImGui::BeginTable("##productsTable", (int)columnInfo.size(), tbflags)) {
    for (auto &ci : columnInfo) ci.setup();

    renderTableHeader();
    renderTableNewRow();
    for (int j = 0; j < (int)db.productTemplate.size(); j++) {
      ImGui::TableNextRow();
      renderTableRow(j, -1);
    }
    for (int j = 0; j < (int)newProd.size(); j++) {
      ImGui::TableNextRow();
      renderTableRow(j, (int)db.productTemplate.size());
    }

    ImGui::EndTable();
  }

  if (deleteIdx >= 0) {
    if (deleteOff == 0) {
      auto org = db.productTemplate;
      db.productTemplate.erase(db.productTemplate.begin() + deleteIdx);
      if (db.saveProducts()) {
        countAtOpen = (int)db.productTemplate.size();
      } else {
        db.productTemplate = org;
      }
    } else {
      newProd.erase(newProd.begin() + deleteIdx);
    }
    deleteIdx = deleteOff = -1;
  }


  bool allowSave = true;
  bool isEdited = (newProd.size() != 0);
  Map<String, int> existingPr;
  char tmpFmt[32];
  for (auto &p : db.productTemplate) {
    if (!p->isValid()) allowSave = false;
    if (p->isEdited()) isEdited = true;

    SPRINTF(tmpFmt, "%0.4f %s", p->unitCoefEdit, unitNames[p->unitNameEdit]);
    auto fullPr = p->nameEdit.getText() + " " + tmpFmt;
    existingPr[fullPr]++;
    if (existingPr[fullPr] > 1) allowSave = false;
  }
  for (auto &p : newProd) {
    if (!p->isValid()) allowSave = false;

    SPRINTF(tmpFmt, "%0.4f %s", p->unitCoefEdit, unitNames[p->unitNameEdit]);
    auto fullPr = p->nameEdit.getText() + " " + tmpFmt;
    existingPr[fullPr]++;
    if (existingPr[fullPr] > 1) allowSave = false;
  }

  bool disableSaveBtn = !isEdited || !allowSave;
  if (disableSaveBtn) {
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_Button]);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_Button]);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
  }
  if (ImGui::Button("Save##products") && !disableSaveBtn) {
    auto org = db.productTemplate;
    db.productTemplate.insert(db.productTemplate.end(), newProd.begin(), newProd.end());

    if (db.saveProducts()) {
      countAtOpen = (int)db.productTemplate.size();
      newProd.clear();
    } else {
      db.productTemplate = org;
    }
  }
  if (disableSaveBtn) {
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
  }
  if (ImGui::IsItemHovered()) {
    if (allowSave) {
      if (!isEdited) {
        ImTooltip(u8"Няма промени");
      }
    } else {
      String err;
      for (auto &[pr, cnt] : existingPr) {
        if (cnt <= 1) continue;
        err += u8"Продукт " + pr + u8" се повтаря\n";
      }
      if (err.length()) ImTooltip(Color_Red, err.c_str());
      else ImTooltip(Color_Red, u8"Невалиди данни");
    }
  }
}
