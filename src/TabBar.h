#pragma once

#include <QTabBar>

#include "core/macros.h"

class QMouseEvent;
class QWindow;
class FakeWindow;

class TabBar : public QTabBar {
  Q_OBJECT
  DISABLE_COPY(TabBar)

 public:
  TabBar(QWidget* parent);
  ~TabBar() = default;
  DEFAULT_MOVE(TabBar)

  void startMovingTab(const QPoint& tabPos);

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

signals:
  // Detach Tab
  void onDetachTabStarted(int index, const QPoint& startPoint);
  void onDetachTabEntered(const QPoint& enterPoint);
  void onDetachTabFinished(const QPoint& newWindowPos, bool isFloating);

 private:
  QPoint m_dragStartPos;
  bool m_dragInitiated;
  FakeWindow* m_fakeWindow;
  bool m_isGrabbingMouse;
  TabBar* m_sourceTabBar;
  QPoint m_offsetFromWindow;

  void finishDrag();
  void grabMouse();
};
