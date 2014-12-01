#include <QFileDialog>
#include <QMainWindow>
#include <QApplication>

#include "API.h"
#include "MainWindow.h"
#include "STabWidget.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), m_tabbar(new STabWidget(this)) {
  qDebug("creating MainWindow");

  this->setWindowTitle(QObject::tr("SilkEdit"));

  setCentralWidget(m_tabbar);
}

MainWindow* MainWindow::create(QWidget* parent, Qt::WindowFlags flags) {
  MainWindow* window = new MainWindow(parent, flags);
  s_windows.append(window);
  return window;
}

void MainWindow::show() {
  QMainWindow::show();
  QApplication::setActiveWindow(this);
}

QList<MainWindow*> MainWindow::s_windows;
