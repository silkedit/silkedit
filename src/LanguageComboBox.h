#pragma once

#include "core/macros.h"
#include "SComboBox.h"

class LanguageComboBox : public SComboBox {
  DISABLE_COPY(LanguageComboBox)

 public:
  LanguageComboBox(QWidget* parent = nullptr);
  ~LanguageComboBox() = default;
  DEFAULT_MOVE(LanguageComboBox)

 private:
};
