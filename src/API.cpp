#include <QApplication>

#include "API.h"
#include "STabWidget.h"
#include "MainWindow.h"

TextEditView* API::activeEditView() {
  MainWindow* window = activeWindow();
  if (window) {
    return window->activeTabWidget()->activeEditView();
  } else {
    return nullptr;
  }
}

STabWidget* API::activeTabWidget() {
  MainWindow* window = activeWindow();
  if (window) {
    return window->activeTabWidget();
  } else {
    return nullptr;
  }
}

MainWindow* API::activeWindow() {
  MainWindow* window = qobject_cast<MainWindow*>(QApplication::activeWindow());
  if (window) {
    return window;
  } else {
    qWarning("active window is null");
    return nullptr;
  }
}

QList<MainWindow*> API::windows() {
  return MainWindow::windows();
}
