#include <QDebug>

#include "DialogButtonBox.h"

DialogButtonBox::~DialogButtonBox() {
  qDebug() << "~DialogButtonBox";
}

QPushButton *DialogButtonBox::button(QDialogButtonBox::StandardButton which) const
{
  return QDialogButtonBox::button(which);
}
