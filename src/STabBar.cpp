#include <QDebug>
#include <QMouseEvent>
#include <QApplication>

#include "STabBar.h"
#include "STabWidget.h"
#include "FakeWindow.h"

STabBar::STabBar(QWidget* parent) : QTabBar(parent), m_fakeWindow(nullptr) {
  setAcceptDrops(true);

  setElideMode(Qt::ElideRight);
  setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);

  setMovable(true);
  setDocumentMode(true);
  setTabsClosable(true);
}

void STabBar::startMovingTab(const QPoint& tabPos, const QPoint& currentPos) {
  qDebug() << "startMovingTab. tabPos:" << tabPos << ", dragStartPos:" << m_dragStartPos
           << ", currentPos" << currentPos;

  QMouseEvent mousePressEvent(
      QEvent::MouseButtonPress, tabPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  QTabBar::mousePressEvent(&mousePressEvent);

  if (currentPos.x() > m_dragStartPos.x()) {
    for (int i = m_dragStartPos.x(); i < currentPos.x(); i++) {
      QMouseEvent startMoveEvent(QEvent::MouseMove,
                                 QPoint(i, m_dragStartPos.y()),
                                 Qt::NoButton,
                                 Qt::LeftButton,
                                 Qt::NoModifier);
      QTabBar::mouseMoveEvent(&startMoveEvent);
    }
  } else {
    for (int i = m_dragStartPos.x(); i > currentPos.x(); i--) {
      QMouseEvent startMoveEvent(QEvent::MouseMove,
                                 QPoint(i, m_dragStartPos.y()),
                                 Qt::NoButton,
                                 Qt::LeftButton,
                                 Qt::NoModifier);
      QTabBar::mouseMoveEvent(&startMoveEvent);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
void STabBar::mousePressEvent(QMouseEvent* event) {
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
void STabBar::mouseMoveEvent(QMouseEvent* event) {
  //  qDebug() << "mouseMoveEvent. pos:" << event->pos() << "globalPos:" << event->globalPos();
  //  qDebug() << "mouseMoveEvent" << event;

  // first mouse enter event after dragging
  if (m_dragInitiated && geometry().contains(event->pos())) {
    qDebug("first mouse enter event");
    m_dragInitiated = false;
    releaseMouse();
    m_fakeWindow->close();
    m_fakeWindow = nullptr;

    // Start mouse move
    emit onDetachTabEntered(event->pos());

    return QTabBar::mouseMoveEvent(event);
  }

  if (m_fakeWindow) {
    m_fakeWindow->moveWithOffset(event->globalPos());
    return QTabBar::mouseMoveEvent(event);
  }

  //  qDebug() << "manhattanLength? " << ((event->pos() - m_dragStartPos).manhattanLength() <
  //  QApplication::startDragDistance()) << "outside of tabbar? " <<
  //  !geometry().contains(event->pos());
  if (!m_dragStartPos.isNull() && ((event->buttons() & Qt::LeftButton)) &&
      ((event->pos() - m_dragStartPos).manhattanLength() > QApplication::startDragDistance()) &&
      (!geometry().contains(event->pos()))) {
    m_dragInitiated = true;
    // Stop the move to be able to convert to a drag
    QMouseEvent* finishMoveEvent = new QMouseEvent(
        QEvent::MouseMove, event->pos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QTabBar::mouseMoveEvent(finishMoveEvent);
    delete finishMoveEvent;
    finishMoveEvent = NULL;

    // Initiate Drag
    qDebug("start dragging a tab");
    grabMouse();
    m_fakeWindow = new FakeWindow(this, m_dragStartPos);
    m_fakeWindow->show();
    emit onDetachTabStarted(tabAt(m_dragStartPos), event->screenPos().toPoint());
  } else {
    QTabBar::mouseMoveEvent(event);
  }
}

void STabBar::mouseReleaseEvent(QMouseEvent* event) {
  qDebug() << "mouseReleaseEvent."
           << "m_dragInitiated:" << m_dragInitiated
           << ", left button:" << (event->button() == Qt::LeftButton);
  if (!m_dragInitiated || event->button() != Qt::LeftButton) {
    QTabBar::mouseReleaseEvent(event);
    return;
  }

  releaseMouse();
  qDebug() << "tabAt(m_dragStartPos):" << tabAt(m_dragStartPos)
           << "m_dragStartPos:" << m_dragStartPos;
  m_fakeWindow->close();
  m_fakeWindow = nullptr;
  emit onDetachTabFinished(event->screenPos().toPoint());
}
