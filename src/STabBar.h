#pragma once

#include <QTabBar>

#include "macros.h"

class QMainWindow;

class STabBar : public QTabBar {
  Q_OBJECT
  DISABLE_COPY(STabBar)

 public:
  STabBar(QWidget* parent);
  ~STabBar() = default;
  DEFAULT_MOVE(STabBar)

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

signals:
  // Detach Tab
  void OnDetachTab(int index, QPoint& dropPoint);
  // Move Tab
  void OnMoveTab(int fromIndex, int toIndex);

 private:
  QPoint m_dragStartPos;
  QPoint m_dragMovedPos;
  QPoint m_dragDropedPos;
  bool m_dragInitiated;
  int m_dragCurrentIndex;
};
