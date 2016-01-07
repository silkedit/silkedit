#pragma once

#include <node.h>
#include <QDialogButtonBox>

#include "core/macros.h"

class NODE_EXTERN DialogButtonBox : public QDialogButtonBox {
  Q_OBJECT
  DISABLE_COPY(DialogButtonBox)

 public:

  Q_INVOKABLE DialogButtonBox(QWidget* parent = 0) : QDialogButtonBox(parent) {}
  ~DialogButtonBox();
  DEFAULT_MOVE(DialogButtonBox)

 public slots:
  QPushButton* button(QDialogButtonBox::StandardButton which) const;
};
