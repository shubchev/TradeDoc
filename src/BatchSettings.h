#include "common.h"

#include "TradeDatabase.h"


class BatchSettings {
protected:
  TradeDB &db;
  CalendarDate today;
  Color cTextColor = Color_White;
  int deleteIdx = -1;
  int deleteOff = -1;
  int countAtOpen = 0;

  Array<BatchTemplate> newBatches;

  class TableColumn {
  public:
    TableColumn(const String &name, int flags, int idx);
    String name;
    String id;
    int flags = 0;
    void setup();
  };
  Array<TableColumn> columnInfo;

  void renderTableHeader();
  void renderTableRow(int row, int offset);
  void renderTableNewButton();

public:
  BatchSettings(TradeDB &db);

  void open();
  void close();

  void render();
};