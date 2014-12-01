// Qt includes
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

//////////////////////////////////////////////////////////////////////////////
void STabBar::mousePressEvent(QMouseEvent* event) {
  qDebug() << "mousePressEvent";
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
  if (m_fakeWindow) {
    m_fakeWindow->moveWithOffset(event->globalPos());
    return QTabBar::mouseMoveEvent(event);
  }

  // Distinguish a drag
  if (!m_dragStartPos.isNull() &&
      ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance())) {
    m_dragInitiated = true;
  }

  // The left button is pressed
  // And the move could also be a drag
  // And the mouse moved outside the tab bar
  if (((event->buttons() & Qt::LeftButton)) && m_dragInitiated &&
      (!geometry().contains(event->pos()))) {
    // Stop the move to be able to convert to a drag
    {
      QMouseEvent* finishMoveEvent = new QMouseEvent(
          QEvent::MouseMove, event->pos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
      QTabBar::mouseMoveEvent(finishMoveEvent);
      delete finishMoveEvent;
      finishMoveEvent = NULL;
    }

    // Initiate Drag
    qDebug("start dragging a tab");
    grabMouse();
    m_fakeWindow = new FakeWindow(this, m_dragStartPos);
    m_fakeWindow->show();
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
  emit OnDetachTab(tabAt(m_dragStartPos), event->screenPos().toPoint());
  m_fakeWindow->close();
  m_fakeWindow = nullptr;
}
