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

static inline ImGuiInputTextFlags InputScalar_DefaultCharsFilter1(ImGuiDataType data_type, const char *format) {
  if (data_type == ImGuiDataType_Float || data_type == ImGuiDataType_Double)
    return ImGuiInputTextFlags_CharsScientific;
  const char format_last_char = format[0] ? format[strlen(format) - 1] : 0;
  return (format_last_char == 'x' || format_last_char == 'X') ? ImGuiInputTextFlags_CharsHexadecimal : ImGuiInputTextFlags_CharsDecimal;
}

// Note: p_data, p_step, p_step_fast are _pointers_ to a memory address holding the data. For an Input widget, p_step and p_step_fast are optional.
// Read code of e.g. InputFloat(), InputInt() etc. or examples in 'Demo->Widgets->Data Types' to understand how to use this function directly.
bool ImInputScalar(const char *label, ImGuiDataType data_type, void *p_data, const void *p_step, const void *p_step_fast, const char *format, ImGuiInputTextFlags flags) {
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImGuiContext &g = *GImGui;
  ImGuiStyle &style = g.Style;

  if (format == NULL)
    format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;

  char buf[64];
  ImGui::DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, p_data, format);

  // Testing ActiveId as a minor optimization as filtering is not needed until active
  if (g.ActiveId == 0 && (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsScientific)) == 0)
    flags |= InputScalar_DefaultCharsFilter1(data_type, format);
  flags |= ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoMarkEdited; // We call MarkItemEdited() ourselves by comparing the actual data rather than the string.

  bool value_changed = false;
  if (p_step != NULL) {
    const float button_size = ImGui::GetFrameHeight();

    ImGui::BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
    ImGui::PushID(label);
    ImGui::SetNextItemWidth(ImMax(1.0f, ImGui::CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
    if (ImGui::InputText("", buf, IM_ARRAYSIZE(buf), flags)) // PushId(label) + "" gives us the expected ID from outside point of view
      value_changed = ImGui::DataTypeApplyFromText(buf, data_type, p_data, format);
    if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
      if (!strlen(buf)) {
        ImGui::DataTypeApplyFromText("0", data_type, p_data, format);
        value_changed = true;
      }
    }
    // Step buttons
    const ImVec2 backup_frame_padding = style.FramePadding;
    style.FramePadding.x = style.FramePadding.y;
    ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;
    if (flags & ImGuiInputTextFlags_ReadOnly)
      ImGui::BeginDisabled();
    ImGui::SameLine(0, style.ItemInnerSpacing.x);
    if (ImGui::ButtonEx("-", ImVec2(button_size, button_size), button_flags)) {
      ImGui::DataTypeApplyOp(data_type, '-', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
      value_changed = true;
    }
    ImGui::SameLine(0, style.ItemInnerSpacing.x);
    if (ImGui::ButtonEx("+", ImVec2(button_size, button_size), button_flags)) {
      ImGui::DataTypeApplyOp(data_type, '+', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
      value_changed = true;
    }
    if (flags & ImGuiInputTextFlags_ReadOnly)
      ImGui::EndDisabled();

    const char *label_end = ImGui::FindRenderedTextEnd(label);
    if (label != label_end) {
      ImGui::SameLine(0, style.ItemInnerSpacing.x);
      ImGui::TextEx(label, label_end);
    }
    style.FramePadding = backup_frame_padding;

    ImGui::PopID();
    ImGui::EndGroup();
  } else {
    if (ImGui::InputText(label, buf, IM_ARRAYSIZE(buf), flags))
      value_changed = ImGui::DataTypeApplyFromText(buf, data_type, p_data, format);
  }
  if (value_changed)
    ImGui::MarkItemEdited(g.LastItemData.ID);

  return value_changed;
}

bool ImInputInt(const char *label, int *v, int step, int step_fast, ImGuiInputTextFlags flags) {
  // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
  const char *format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
  return ImInputScalar(label, ImGuiDataType_S32, (void *)v, (void *)(step > 0 ? &step : NULL), (void *)(step_fast > 0 ? &step_fast : NULL), format, flags);
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
