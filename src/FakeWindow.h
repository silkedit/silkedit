#pragma once

#include <QWidget>

#include "macros.h"

class STabBar;

class FakeWindow : public QWidget {
  Q_OBJECT
  DISABLE_COPY(FakeWindow)

 public:
  FakeWindow(STabBar* tabbar, const QPoint& pos);
  ~FakeWindow();
  DEFAULT_MOVE(FakeWindow)

  void moveWithOffset(const QPoint& pos);

 private:
  QPoint m_offset;
};
