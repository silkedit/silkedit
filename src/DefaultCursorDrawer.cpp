#include <QRect>
#include <QColor>

#include "DefaultCursorDrawer.h"

std::tuple<QRect, QColor> DefaultCursorDrawer::draw(const QRect& cursorRect) {
  QRect r = cursorRect;
  r.setWidth(cursorWidth());
  return std::make_tuple(r, QColor("red"));
}
