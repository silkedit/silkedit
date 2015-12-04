#pragma once

#include "core/macros.h"
#include "ComboBox.h"

namespace core {
class BOM;
}

class BOMComboBox : public ComboBox {
  DISABLE_COPY(BOMComboBox)
 public:
  BOMComboBox(QWidget* parent = nullptr);
  ~BOMComboBox() = default;
  DEFAULT_MOVE(BOMComboBox)

  void setCurrentBOM(const core::BOM& bom);
  core::BOM currentBOM();
};
