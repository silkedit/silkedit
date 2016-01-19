#include <QDebug>

#include "VBoxLayout.h"

VBoxLayout::VBoxLayout() : QVBoxLayout() {}

VBoxLayout::VBoxLayout(QWidget* parent) : QVBoxLayout(parent) {}

VBoxLayout::~VBoxLayout()
{
  qDebug() << "~VBoxLayout";
}

void VBoxLayout::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment) {
  QVBoxLayout::addWidget(widget, stretch, alignment);
}
