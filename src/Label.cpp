#include <QDebug>

#include "Label.h"

Label::Label(QWidget* parent, Qt::WindowFlags f) : QLabel(parent, f) {
  qDebug() << "Label()";
}

Label::Label(const QString& text, QWidget* parent, Qt::WindowFlags f) : QLabel(text, parent, f) {
  qDebug() << "Label(" << text << ")";
}

Label::~Label() {
  qDebug() << "~Label";
}
