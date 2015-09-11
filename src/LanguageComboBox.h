#pragma once

#include <QComboBox>

#include "core/macros.h"

class LanguageComboBox : public QComboBox {
  DISABLE_COPY(LanguageComboBox)

 public:
  LanguageComboBox(QWidget* parent = nullptr);
  ~LanguageComboBox() = default;
  DEFAULT_MOVE(LanguageComboBox)

 private:
};
