#include <QApplication>
#include <QMessageBox>

#include "API.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "MainWindow.h"
#include "MenuService.h"

TextEditView* API::activeEditView() {
  TabView* tabView = activeTabView();
  if (tabView) {
    return tabView->activeEditView();
  } else {
    qDebug("active tab view is null");
    return nullptr;
  }
}

TabView* API::activeTabView(bool createIfNull) {
  TabViewGroup* tabViewGroup = activeTabViewGroup();
  if (tabViewGroup) {
    return tabViewGroup->activeTab(createIfNull);
  } else {
    qDebug("active tab view group is null");
    return nullptr;
  }
}

TabViewGroup* API::activeTabViewGroup() {
  MainWindow* window = activeWindow();
  if (window) {
    return window->tabViewGroup();
  } else {
    qDebug("active window is null");
    return nullptr;
  }
}

MainWindow* API::activeWindow() { return qobject_cast<MainWindow*>(QApplication::activeWindow()); }

QList<MainWindow*> API::windows() { return MainWindow::windows(); }

void API::hideActiveFindReplacePanel() {
  if (MainWindow* window = activeWindow()) {
    window->hideFindReplacePanel();
  }
}

void API::showDialog(const QString& msg) {
  QMessageBox msgBox;
  msgBox.setText(msg);
  msgBox.exec();
}

void API::loadMenu(const std::string& ymlPath) { MenuService::loadMenu(ymlPath); }
