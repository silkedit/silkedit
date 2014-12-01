#include <QDebug>

#include "FakeWindow.h"
#include "STabBar.h"

FakeWindow::FakeWindow(STabBar* tabbar, const QPoint& pos) {
  qDebug() << "constructor of FakeWindow. pos:" << pos;
  setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
  setAttribute(Qt::WA_DeleteOnClose);

  QRect rect = tabbar->tabRect(tabbar->currentIndex());
  QPixmap pixmap = tabbar->grab(rect);
  QPalette palette;

  palette.setBrush(this->backgroundRole(), QBrush(pixmap));

  this->setPalette(palette);
  this->setGeometry(rect);
  this->setWindowOpacity(0.5);
  this->setAttribute(Qt::WA_TransparentForMouseEvents);

  qDebug() << "mapToGlobal(pos):" << tabbar->mapToGlobal(pos)
           << "tabbar->window()->pos():" << tabbar->window()->pos();
  m_offset = tabbar->mapToGlobal(pos) - tabbar->window()->pos();
}

FakeWindow::~FakeWindow() {
  qDebug("~FakeWindow");
}

void FakeWindow::moveWithOffset(const QPoint& pos) {
  QWidget::move(pos - m_offset);
}
