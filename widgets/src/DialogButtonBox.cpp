#include <QDebug>

#include "DialogButtonBox.h"

DialogButtonBox::~DialogButtonBox() {
  qDebug() << "~DialogButtonBox";
}

QPushButton *DialogButtonBox::button(int which) const
{
  return QDialogButtonBox::button(static_cast<StandardButton>(which));
}
