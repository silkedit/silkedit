#pragma once

#include "core/macros.h"
#include "ComboBox.h"

class LanguageComboBox : public ComboBox {
  DISABLE_COPY(LanguageComboBox)

 public:
  LanguageComboBox(QWidget* parent = nullptr);
  ~LanguageComboBox() = default;
  DEFAULT_MOVE(LanguageComboBox)

 private:
};
