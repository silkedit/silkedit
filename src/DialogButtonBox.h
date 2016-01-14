#pragma once

#include <QDialogButtonBox>

#include "core/macros.h"

class DialogButtonBox : public QDialogButtonBox {
  Q_OBJECT
  DISABLE_COPY(DialogButtonBox)

 public:

  Q_INVOKABLE DialogButtonBox(QWidget* parent = 0) : QDialogButtonBox(parent) {}
  ~DialogButtonBox();
  DEFAULT_MOVE(DialogButtonBox)

 public slots:
  QPushButton* button(int which) const;
};
