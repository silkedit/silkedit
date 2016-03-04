#pragma once

#include <QItemSelectionModel>

#include "Wrapper.h"

namespace core {

class ItemSelectionModel : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QItemSelectionModel*")

 public:
  enum SelectionFlag {
    NoUpdate = 0x0000,
    Clear = 0x0001,
    Select = 0x0002,
    Deselect = 0x0004,
    Toggle = 0x0008,
    Current = 0x0010,
    Rows = 0x0020,
    Columns = 0x0040,
    SelectCurrent = Select | Current,
    ToggleCurrent = Toggle | Current,
    ClearAndSelect = Clear | Select
  };
  Q_ENUM(SelectionFlag)

  ItemSelectionModel(QItemSelectionModel* model);
  ~ItemSelectionModel() = default;

 public slots:
  void select(const QModelIndex& index, int command);
};

}  // namespace core

Q_DECLARE_METATYPE(core::ItemSelectionModel*)
Q_DECLARE_METATYPE(QItemSelectionModel*)
