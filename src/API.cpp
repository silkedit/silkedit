#include <QApplication>

#include "API.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "MainWindow.h"

TextEditView* API::activeEditView() {
  TabView* tabView = activeTabView();
  if (tabView) {
    return tabView->activeEditView();
  } else {
    qDebug("active tab view is null");
    return nullptr;
  }
}

TabView* API::activeTabView() {
  TabViewGroup* tabViewGroup = activeTabViewGroup();
  if (tabViewGroup) {
    return tabViewGroup->activeTab();
  } else {
    qDebug("active tab view group is null");
    return nullptr;
  }
}

TabViewGroup* API::activeTabViewGroup() {
  MainWindow* window = activeWindow();
  if (window) {
    return window->activeTabViewGroup();
  } else {
    qDebug("active window is null");
    return nullptr;
  }
}

MainWindow* API::activeWindow() {
  return qobject_cast<MainWindow*>(QApplication::activeWindow());
}

QList<MainWindow*> API::windows() {
  return MainWindow::windows();
}
