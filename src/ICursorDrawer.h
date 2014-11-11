#pragma once

#include <tuple>
#include <QString>

#include "macros.h"

class QRect;
class QColor;

class ViEditView;

class ICursorDrawer {
  DISABLE_COPY_AND_MOVE(ICursorDrawer)

 public:
  ICursorDrawer() : m_width(1) {}
  virtual ~ICursorDrawer() = default;

  int cursorWidth() { return m_width; }
  void setCursorWidth(int width) { m_width = width; }

  virtual std::tuple<QRect, QColor> draw(const QRect& cursorRect) = 0;
  virtual void updateCursor(const ViEditView&) {}

 private:
  QString m_name;
  int m_width;
};
