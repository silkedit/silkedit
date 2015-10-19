#include <QDebug>

#include "FakeWindow.h"
#include "TabBar.h"

/**
 * @brief FakeWindow::FakeWindow
 * @param tabbar
 * @param pos drag start position (tabbar relative)
 */
FakeWindow::FakeWindow(TabBar* tabbar, const QPoint& dragStartPos) {
  qDebug() << "constructor of FakeWindow. pos:" << dragStartPos;
  setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
  setAttribute(Qt::WA_DeleteOnClose);
  setAttribute(Qt::WA_ShowWithoutActivating);
  // WA_TransparentForMouseEvents should pass through mouse event but it doens't work as expected on
  // Mac
  // https://bugreports.qt.io/browse/QTBUG-41696
  setAttribute(Qt::WA_TransparentForMouseEvents);

  QRect rect = tabbar->tabRect(tabbar->currentIndex());
  QPixmap pixmap = tabbar->grab(rect);
  QPalette palette;

  palette.setBrush(this->backgroundRole(), QBrush(pixmap));

  this->setPalette(palette);
  this->setGeometry(rect);
  this->setWindowOpacity(0.5);
  this->setAttribute(Qt::WA_TransparentForMouseEvents);

  qDebug() << "pos:" << dragStartPos << "rect.topLeft:" << rect.topLeft();
  m_offset = dragStartPos - rect.topLeft();
}

FakeWindow::~FakeWindow() {
  qDebug("~FakeWindow");
}

void FakeWindow::moveWithOffset(const QPoint& pos) {
  QWidget::move(pos - m_offset);
}
