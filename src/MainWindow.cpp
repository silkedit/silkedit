#include <QFileDialog>
#include <QMainWindow>
#include <QApplication>
#include <QDebug>

#include "API.h"
#include "MainWindow.h"
#include "STabWidget.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), m_tabWidget(new STabWidget(this)) {
  qDebug("creating MainWindow");

  setWindowTitle(QObject::tr("SilkEdit"));

  setCentralWidget(m_tabWidget);

  QObject::connect(m_tabWidget, &STabWidget::allTabRemoved, [this]() {
    qDebug() << "allTabRemoved";
    if (m_tabWidget->tabDragging()) {
      hide();
    } else {
      close();
    }
  });
}

MainWindow* MainWindow::create(QWidget* parent, Qt::WindowFlags flags) {
  MainWindow* window = new MainWindow(parent, flags);
  window->resize(1280, 720);
  s_windows.append(window);
  return window;
}

MainWindow::~MainWindow() {
  qDebug("~MainWindow");
}

void MainWindow::show() {
  QMainWindow::show();
  QApplication::setActiveWindow(this);
}

void MainWindow::close() {
  if (s_windows.removeOne(this)) {
    deleteLater();
  }
}

QList<MainWindow*> MainWindow::s_windows;
