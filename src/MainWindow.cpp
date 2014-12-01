#include <QFileDialog>
#include <QMenuBar>
#include <QAction>
#include <QMainWindow>
#include <QApplication>

#include "API.h"
#include "MainWindow.h"
#include "CommandAction.h"
#include "STabWidget.h"
#include "commands/OpenFileCommand.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), m_tabbar(new STabWidget(this)) {
  qDebug("creating MainWindow");

  this->setWindowTitle(QObject::tr("SilkEdit"));

  setCentralWidget(m_tabbar);

  auto openFileAction = new CommandAction(tr("&Open..."), OpenFileCommand::name, this);

  auto fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openFileAction);

  m_tabbar->addNew();

  // Set focus to active edit view
  if (auto v = m_tabbar->activeEditView()) {
    v->setFocus();
  }
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
