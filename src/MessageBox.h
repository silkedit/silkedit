#pragma once

#include <QMessageBox>

#include "core/macros.h"

class MessageBox : public QMessageBox {
  Q_OBJECT
  DISABLE_COPY(MessageBox)

 public:
  Q_INVOKABLE explicit MessageBox(QWidget* parent = 0) : QMessageBox(parent) {}
  ~MessageBox() = default;
  DEFAULT_MOVE(MessageBox)
};
