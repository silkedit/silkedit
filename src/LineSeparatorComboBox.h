#pragma once

#include "core/macros.h"
#include "ComboBox.h"

class TextEditView;

class LineSeparatorComboBox : public ComboBox {
  DISABLE_COPY(LineSeparatorComboBox)

 public:
  LineSeparatorComboBox(QWidget* parent = nullptr);
  ~LineSeparatorComboBox() = default;
  DEFAULT_MOVE(LineSeparatorComboBox)

  void setCurrentLineSeparator(const QString& separator);
  QString currentLineSeparator();

 private:
};
