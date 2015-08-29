#pragma once

#include <QComboBox>

#include "macros.h"

class TextEditView;

class LineSeparatorComboBox : public QComboBox {
  DISABLE_COPY(LineSeparatorComboBox)

 public:
  LineSeparatorComboBox(QWidget* parent = nullptr);
  ~LineSeparatorComboBox() = default;
  DEFAULT_MOVE(LineSeparatorComboBox)

  void setCurrentLineSeparator(const QString& separator);
  QString currentLineSeparator();

 private:
};
