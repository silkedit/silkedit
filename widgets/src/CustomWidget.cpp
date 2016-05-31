#include <QStyleOption>
#include <QPainter>

#include "CustomWidget.h"

CustomWidget::CustomWidget(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f) {}

// applying CSS styles to custom widgets inherited from QWidget requires reimplementing
// paintEvent() in this way:
// http://stackoverflow.com/questions/7276330/qt-stylesheet-for-custom-widget
void CustomWidget::paintEvent(QPaintEvent*) {
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
