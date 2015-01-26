#pragma once

#include <QWidget>

#include "macros.h"

class TabBar;

class FakeWindow : public QWidget {
  Q_OBJECT
  DISABLE_COPY(FakeWindow)

 public:
  FakeWindow(TabBar* tabbar, const QPoint& pos);
  ~FakeWindow();
  DEFAULT_MOVE(FakeWindow)

  void moveWithOffset(const QPoint& pos);

 private:
  QPoint m_offset;
  QPoint m_offsetFromWindow;
};
