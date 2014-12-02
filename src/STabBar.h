#pragma once

#include <QTabBar>

#include "macros.h"

class QMouseEvent;
class QMainWindow;
class FakeWindow;

class STabBar : public QTabBar {
  Q_OBJECT
  DISABLE_COPY(STabBar)

 public:
  STabBar(QWidget* parent);
  ~STabBar() = default;
  DEFAULT_MOVE(STabBar)

  void startMovingTab(const QPoint& tabPos, const QPoint& currentPos);

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

signals:
  // Detach Tab
  void onDetachTabStarted(int index, const QPoint& startPoint);
  void onDetachTabEntered(const QPoint& enterPoint);
  void onDetachTabFinished(const QPoint& dropPoint);

 private:
  QPoint m_dragStartPos;
  bool m_dragInitiated;
  FakeWindow* m_fakeWindow;
};
