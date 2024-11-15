#include "common.h"

#include "TradeDatabase.h"


class ProductSettings {
protected:
  TradeDB &db;
  char noFmt[32] = { 0 };
  int deleteIdx = -1;
  int deleteOff = 0;
  int countAtOpen = 0;

  Array<ProductTemplate> newProd;

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
  void renderTableNewRow();

public:
  ProductSettings(TradeDB &db);

  void open();
  void close();

  void render();
};