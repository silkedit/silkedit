#include <QDebug>
#include <QMouseEvent>
#include <QApplication>

#include "TabBar.h"
#include "TabWidget.h"
#include "FakeWindow.h"
#include "API.h"
#include "MainWindow.h"

TabBar::TabBar(QWidget* parent)
    : QTabBar(parent), m_fakeWindow(nullptr), m_isGrabbingMouse(false) {
  setAcceptDrops(true);

  setElideMode(Qt::ElideRight);
  setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);

  setMovable(true);
  setDocumentMode(true);
  setTabsClosable(true);
}

void TabBar::startMovingTab(const QPoint& tabPos) {
  qDebug() << "startMovingTab. tabPos:" << tabPos;

  QMouseEvent pressEvent(
      QEvent::MouseButtonPress, tabPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  QTabBar::mousePressEvent(&pressEvent);

  QMouseEvent startMoveEvent(QEvent::MouseMove,
                             QPoint(tabPos.x(), tabPos.y()),
                             Qt::NoButton,
                             Qt::LeftButton,
                             Qt::NoModifier);
  QTabBar::mouseMoveEvent(&startMoveEvent);

  m_isGrabbingMouse = true;
  m_dragStartPos = tabPos;
}

//////////////////////////////////////////////////////////////////////////////
void TabBar::mousePressEvent(QMouseEvent* event) {
  qDebug() << "mousePressEvent."
           << "pos:" << event->pos();
  if (event->button() == Qt::LeftButton && tabAt(event->pos()) >= 0) {
    qDebug() << "m_dragStartPos is set at:" << event->pos();
    m_dragStartPos = event->pos();
  }
  m_dragInitiated = false;

  QTabBar::mousePressEvent(event);
}

//////////////////////////////////////////////////////////////////////////////
void TabBar::mouseMoveEvent(QMouseEvent* event) {
  // If we call winId() here, drag no longer works.
  //  qDebug() << "mouseMoveEvent. pos:" << event->pos() << "globalPos:" << event->globalPos();
  //    qDebug() << "mouseMoveEvent" << event;

  // first mouse enter event after dragging
  if (m_dragInitiated && geometry().contains(event->pos())) {
    qDebug("first mouse enter event");
    finishDrag();

    // Start mouse move
    emit onDetachTabEntered(event->screenPos().toPoint());
    return QTabBar::mouseMoveEvent(event);
  }

  // dragging tab is over an another tab bar.
  QWidget* widgetUnderMouse =
      QApplication::widgetAt(event->screenPos().x(), event->screenPos().y());
  TabBar* anotherTabBar = qobject_cast<TabBar*>(widgetUnderMouse);

  if (m_dragInitiated && anotherTabBar && anotherTabBar != this) {
    qDebug("dragging tab is over an another tab bar.");

    finishDrag();
    emit anotherTabBar->onDetachTabEntered(event->screenPos().toPoint());
    anotherTabBar->grabMouse();
    return;
  }

  if (m_dragInitiated && m_fakeWindow) {
    m_fakeWindow->moveWithOffset(event->globalPos());
    return QTabBar::mouseMoveEvent(event);
  }

  //    qDebug() << "manhattanLength? " << ((event->pos() - m_dragStartPos).manhattanLength() <
  //    QApplication::startDragDistance()) << "outside of tabbar? " <<
  //    !geometry().contains(event->pos());
  if (!m_dragStartPos.isNull() && ((event->buttons() & Qt::LeftButton)) &&
      ((event->pos() - m_dragStartPos).manhattanLength() > QApplication::startDragDistance()) &&
      (!geometry().contains(event->pos()))) {
    m_dragInitiated = true;
    // Stop the move to be able to convert to a drag
    QMouseEvent finishMoveEvent(
        QEvent::MouseMove, event->pos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QTabBar::mouseMoveEvent(&finishMoveEvent);

    // Initiate Drag
    qDebug("start dragging a tab");
    m_fakeWindow = new FakeWindow(this, m_dragStartPos);
    m_fakeWindow->show();
    emit onDetachTabStarted(tabAt(m_dragStartPos), event->screenPos().toPoint());
  } else {
    QTabBar::mouseMoveEvent(event);
  }
}

void TabBar::mouseReleaseEvent(QMouseEvent* event) {
  qDebug() << "mouseReleaseEvent."
           << "m_dragInitiated:" << m_dragInitiated
           << ", left button:" << (event->button() == Qt::LeftButton);
  if (m_isGrabbingMouse) {
    qDebug("releaseMouse() called.");
    m_isGrabbingMouse = false;
    releaseMouse();
  }

  if (!m_dragInitiated || event->button() != Qt::LeftButton) {
    QTabBar::mouseReleaseEvent(event);
    return;
  }

  finishDrag();
  emit onDetachTabFinished(event->screenPos().toPoint());
  QTabBar::mouseReleaseEvent(event);
}

void TabBar::finishDrag() {
  m_dragInitiated = false;
  m_dragStartPos = QPoint();
  if (m_fakeWindow) {
    m_fakeWindow->close();
    m_fakeWindow = nullptr;
  }
}

void TabBar::grabMouse() {
  QTabBar::grabMouse();
  m_isGrabbingMouse = true;
}
