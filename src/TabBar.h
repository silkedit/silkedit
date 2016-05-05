#pragma once

#include <QTabBar>

#include "core/macros.h"

class QMouseEvent;
class QWindow;
class FakeWindow;
namespace core {
    class Theme;
}


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
  void leaveEvent(QEvent* event) override;
  void tabInserted(int index) override;
  void tabRemoved(int index) override;
  void contextMenuEvent(QContextMenuEvent * event) override;

signals:
  // Detach Tab
  void onMousePress(int index);
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
  void hideAllCloseButtons();
  void showCloseButtonOnActiveTab(const QPoint& pos);
  void setTheme(const core::Theme* theme);
};
