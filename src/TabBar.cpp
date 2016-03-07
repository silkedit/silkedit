#include <QDebug>
#include <QMouseEvent>
#include <QApplication>
#include <QMenu>

#include "TabBar.h"
#include "TabView.h"
#include "FakeWindow.h"
#include "Window.h"
#include "App.h"
#include "CommandAction.h"
#include "core/Config.h"
#include "core/Theme.h"
#include "core/Util.h"

using core::Config;
using core::Theme;
using core::Util;
using core::ColorSettings;

TabBar::TabBar(QWidget* parent)
    : QTabBar(parent), m_fakeWindow(nullptr), m_isGrabbingMouse(false), m_sourceTabBar(nullptr) {
  setTheme(Config::singleton().theme());
  setAcceptDrops(true);
  setElideMode(Qt::ElideRight);
  setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
  setMouseTracking(true);
  setUsesScrollButtons(true);
  hideAllCloseButtons();

  connect(&Config::singleton(), &Config::themeChanged, this, &TabBar::setTheme);
}

void TabBar::setTheme(const Theme* theme) {
  qDebug("TabBar theme is changed");
  if (!theme) {
    qWarning("theme is null");
    return;
  }

  if (theme->tabBarSettings != nullptr) {
    QString style;
    ColorSettings* tabBarSettings = theme->tabBarSettings.get();

    style = QString(
                "TabBar::tab {"
                "background-color: %1;"
                "color: %2;"
                "}")
                .arg(Util::qcolorForStyleSheet(tabBarSettings->value("background")))
                .arg(Util::qcolorForStyleSheet(tabBarSettings->value("foreground")));

    style += QString(
                 "TabBar::tab:selected {"
                 "background-color: %1;"
                 "color: %2;"
                 "border-left: 2px solid;"
                 "border-color: %3;"
                 "}")
                 .arg(Util::qcolorForStyleSheet(tabBarSettings->value("selected")))
                 .arg(Util::qcolorForStyleSheet(tabBarSettings->value("foreground")))
                 .arg(Util::qcolorForStyleSheet(tabBarSettings->value("selectedBorder")));

    this->setStyleSheet(style);
  }
}

void TabBar::startMovingTab(const QPoint& tabPos) {
  qDebug() << "startMovingTab. tabPos:" << tabPos;

  QMouseEvent pressEvent(QEvent::MouseButtonPress, tabPos, Qt::LeftButton, Qt::LeftButton,
                         Qt::NoModifier);
  QTabBar::mousePressEvent(&pressEvent);

  QMouseEvent startMoveEvent(QEvent::MouseMove, QPoint(tabPos.x(), tabPos.y()), Qt::NoButton,
                             Qt::LeftButton, Qt::NoModifier);
  QTabBar::mouseMoveEvent(&startMoveEvent);

  m_isGrabbingMouse = true;
  m_dragStartPos = tabPos;
}

void TabBar::mousePressEvent(QMouseEvent* event) {
  qDebug() << "mousePressEvent."
           << "pos:" << event->pos();
  if (event->button() == Qt::LeftButton && tabAt(event->pos()) >= 0) {
    qDebug() << "m_dragStartPos is set at:" << event->pos();
    m_dragStartPos = event->pos();
    QPoint offset = m_dragStartPos - tabRect(tabAt(event->pos())).topLeft();
    m_offsetFromWindow = offset + mapToGlobal(tabRect(0).topLeft()) - window()->pos();
  }
  m_dragInitiated = false;

  QTabBar::mousePressEvent(event);
}

void TabBar::showCloseButtonOnActiveTab(const QPoint& pos) {
  hideAllCloseButtons();

  int index = tabAt(pos);
  if (index >= 0) {
    if (auto w = tabButton(index, QTabBar::RightSide)) {
      w->show();
    }
  }
}

void TabBar::mouseMoveEvent(QMouseEvent* event) {
  // while dragging
  if (event->buttons() & Qt::LeftButton) {
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
    TabBar* anotherTabBar = App::tabBarAt(event->screenPos().x(), event->screenPos().y());

    if (m_dragInitiated && anotherTabBar && anotherTabBar != this) {
      qDebug("dragging tab is over an another tab bar.");

      finishDrag();
      emit anotherTabBar->onDetachTabEntered(event->screenPos().toPoint());
      anotherTabBar->m_sourceTabBar = this;
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
      QMouseEvent finishMoveEvent(QEvent::MouseMove, event->pos(), Qt::NoButton, Qt::NoButton,
                                  Qt::NoModifier);
      QTabBar::mouseMoveEvent(&finishMoveEvent);

      // Initiate Drag
      qDebug("start dragging a tab");
      m_fakeWindow = new FakeWindow(this, m_dragStartPos);
      m_fakeWindow->show();
      emit onDetachTabStarted(tabAt(m_dragStartPos), event->screenPos().toPoint());
    } else {
      QTabBar::mouseMoveEvent(event);
    }
    // not dragging
  } else {
    showCloseButtonOnActiveTab(event->pos());
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

  QPoint newWindowPos = event->screenPos().toPoint() - m_offsetFromWindow;
  if (m_sourceTabBar) {
    qDebug("emit onDetachTabFinished from a source TabBar");
    emit m_sourceTabBar->onDetachTabFinished(newWindowPos, false);
    m_sourceTabBar = nullptr;
  } else {
    qDebug("m_sourceTabBar is null");
  }

  if (!m_dragInitiated || event->button() != Qt::LeftButton) {
    QTabBar::mouseReleaseEvent(event);
    return;
  }

  finishDrag();
  emit onDetachTabFinished(newWindowPos, true);
  QTabBar::mouseReleaseEvent(event);
}

void TabBar::leaveEvent(QEvent*) {
  hideAllCloseButtons();
}

void TabBar::tabInserted(int index) {
  if (auto w = tabButton(index, QTabBar::RightSide)) {
    w->hide();
  }
}

void TabBar::tabRemoved(int) {
  showCloseButtonOnActiveTab(mapFromGlobal(QCursor::pos()));
}

void TabBar::contextMenuEvent(QContextMenuEvent* event) {
  auto index = tabAt(event->pos());

  // context menu on tab bar
  if (index == -1) {
    return;
  }

  // context menu on an each tab
  QMenu menu;
  menu.addAction(new CommandAction("close_tab", tr("Close"), "close_tab",
                                   &menu, CommandArgument{{"index", QVariant(index)}}));
  menu.addAction(new CommandAction("close_other_tabs", tr("Close Other Tabs"), "close_other_tabs",
                                   &menu, CommandArgument{{"index", QVariant(index)}}));
  menu.addAction(new CommandAction("close_tabs_to_the_right", tr("Close Tabs to the Right"), "close_tabs_to_the_right",
                                   &menu, CommandArgument{{"index", QVariant(index)}}));
  menu.exec(event->globalPos());
}

void TabBar::finishDrag() {
  m_dragInitiated = false;
  m_dragStartPos = QPoint();
  m_offsetFromWindow = QPoint();
  if (m_fakeWindow) {
    m_fakeWindow->close();
    m_fakeWindow = nullptr;
  }
}

void TabBar::grabMouse() {
  QTabBar::grabMouse();
  m_isGrabbingMouse = true;
}

void TabBar::hideAllCloseButtons() {
  for (int i = 0; i < count(); i++) {
    if (auto w = tabButton(i, QTabBar::RightSide)) {
      w->hide();
    }
  }
}
