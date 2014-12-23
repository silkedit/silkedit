#pragma once

#include <QComboBox>

#include "macros.h"

class LanguageComboBox : public QComboBox {
  DISABLE_COPY(LanguageComboBox)

 public:
  LanguageComboBox(QWidget* parent = nullptr);
  ~LanguageComboBox() = default;
  DEFAULT_MOVE(LanguageComboBox)

 private:
};
