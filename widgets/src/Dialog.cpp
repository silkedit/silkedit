#include <QDebug>
#include "Dialog.h"

Dialog::Dialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f) {}

Dialog::~Dialog() {
  qDebug() << "~Dialog";
}

void Dialog::resize(int w, int h)
{
  QDialog::resize(w, h);
}

void Dialog::setGeometry(int x, int y, int w, int h)
{
  QDialog::setGeometry(x, y, w, h);
}

void Dialog::setLayout(QLayout *layout)
{
  QDialog::setLayout(layout);
}
