#include "BatchSettings.h"

void ImTooltip(const Color &color, const char *fmt, ...) {
  ImGui::BeginTooltip();
  va_list vl;
  va_start(vl, fmt);
  ImGui::TextColoredV(color, fmt, vl);
  va_end(vl);
  ImGui::EndTooltip();
}
void ImTooltip(const char *fmt, ...) {
  ImGui::BeginTooltip();
  va_list vl;
  va_start(vl, fmt);
  ImGui::TextV(fmt, vl);
  va_end(vl);
  ImGui::EndTooltip();
}

BatchSettings::TableColumn::TableColumn(const String &n, int f, int idx) {
  name = n;
  id = "BatchTableColumn" + toString(idx);
  flags = f;
}

void BatchSettings::TableColumn::setup() {
  ImGui::TableSetupColumn(id.c_str(), flags);
}




BatchSettings::BatchSettings(TradeDB &_db) : db(_db) {
  int idx = 0;
  columnInfo.push_back(TableColumn(u8"", ImGuiTableColumnFlags_WidthFixed, idx++));
  columnInfo.push_back(TableColumn(u8"№", ImGuiTableColumnFlags_WidthFixed, idx++));
  columnInfo.push_back(TableColumn(u8"№ на партида", ImGuiTableColumnFlags_WidthStretch, idx++));
  columnInfo.push_back(TableColumn(u8"Годност", ImGuiTableColumnFlags_WidthFixed, idx++));
}

void BatchSettings::open() {
  countAtOpen = (int)db.batchTemplate.size();
  for (auto &b : db.batchTemplate) b->setupHelpers();
}

void BatchSettings::close() {
  newBatches.clear();
}

void BatchSettings::renderTableHeader() {
  for (auto &ci : columnInfo) {
    ImGui::TableNextColumn();
    ImGui::TextColored(Color_Red, ci.name.c_str());
  }
}

void BatchSettings::renderTableNewButton() {
  ImGui::TableNextColumn();

  ImGui::PushStyleColor(ImGuiCol_Button, vec4(0));
  if (ImGui::SmallButton("+##batch")) {
    auto bi = IBatchTemplate::create();
    if (bi) {
      newBatches.push_back(bi);
    }
  }
  ImGui::PopStyleColor();
}

void BatchSettings::renderTableRow(int row, int offset) {
  BatchTemplate bi;
  if (offset >= 0) bi = newBatches[row];
  else bi = db.batchTemplate[row];
  auto rowStr = toString(row + max(offset, 0));

  {
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Button, vec4(0));
    if (ImGui::SmallButton(("x##batch" + rowStr).c_str())) {
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
    int64_t step = 1;
    ImGui::InputScalar(("##batchNoPick" + rowStr).c_str(), ImGuiDataType_S64,
                        &bi->noEdit, &step, &step);
    bi->isEdited = (bi->noEdit != bi->no);
  }

  {
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(100);
    auto date = bi->expEdit.getSelection();

    auto dDays = date.daysSince(today);
    if (dDays <= 7) {
      if (dDays < 0) bi->expEdit.setButtonTextColor(Color_Red);
      else bi->expEdit.setButtonTextColor(Color_Yellow);
    } else bi->expEdit.setButtonTextColor(cTextColor);
    bi->expEdit.render("batchDatePick" + rowStr);
    if (dDays <= 7) {
      if (ImGui::IsItemHovered()) {
        if (dDays < 0) ImTooltip(u8"Изтекъл срок");
        else ImTooltip(u8"Соркът изтича след < 7 дни");
      }
    }

    bi->isEdited |= (bi->expEdit.getSelection() != bi->exp);
  }
}

void BatchSettings::render() {
  auto &style = ImGui::GetStyle();
  today = CalendarDate::today();
  cTextColor = style.Colors[ImGuiCol_Text];

  ImGuiTableFlags tbflags = ImGuiTableFlags_SizingFixedFit |
                            ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_Borders;
  if (ImGui::BeginTable("##batchesTable", (int)columnInfo.size(), tbflags)) {
    for (auto &ci : columnInfo) ci.setup();

    renderTableHeader();
    renderTableNewButton();
    for (int j = 0; j < (int)db.batchTemplate.size(); j++) {
      ImGui::TableNextRow();
      renderTableRow(j, -1);
    }
    for (int j = 0; j < (int)newBatches.size(); j++) {
      ImGui::TableNextRow();
      renderTableRow(j, (int)db.batchTemplate.size());
    }

    ImGui::EndTable();
  }

  if (deleteIdx >= 0) {
    if (deleteOff == 0) {
      auto orgBatch = db.batchTemplate;
      db.batchTemplate.erase(db.batchTemplate.begin() + deleteIdx);
      if (db.saveBatches()) {
        countAtOpen = (int)db.batchTemplate.size();
      } else {
        db.batchTemplate = orgBatch;
      }
    } else {
      newBatches.erase(newBatches.begin() + deleteIdx);
    }
    deleteIdx = deleteOff = -1;
  }

  bool allowSave = true;
  bool isEdited = (newBatches.size() != 0);
  Map<int64_t, int> existingNo;
  for (auto &b : db.batchTemplate) {
    if (!b->isValid()) allowSave = false;
    existingNo[b->noEdit]++;
    if (existingNo[b->noEdit] > 1) allowSave = false;
    isEdited |= b->isEdited;
  }
  for (auto &b : newBatches) {
    if (!b->isValid()) allowSave = false;
    existingNo[b->noEdit]++;
    if (existingNo[b->noEdit] > 1) allowSave = false;
  }

  bool disableSaveBtn = !isEdited || !allowSave;
  if (disableSaveBtn) {
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_Button]);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_Button]);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
  }
  if (ImGui::Button("Save##batches") && !disableSaveBtn) {
    auto orgBatch = db.batchTemplate;
    db.batchTemplate.insert(db.batchTemplate.end(), newBatches.begin(), newBatches.end());

    if (db.saveBatches() && db.saveProducts()) {
      countAtOpen = (int)db.batchTemplate.size();
      newBatches.clear();
    } else {
      db.batchTemplate = orgBatch;
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
      for (auto &[no, cnt] : existingNo) {
        if (cnt <= 1) continue;
        err += u8"Партида № " + toString(no) + u8" се повтаря\n";
      }
      if (err.length()) ImTooltip(Color_Red, err.c_str());
      else ImTooltip(Color_Red, u8"Невалиди данни");
    }
  }
}
