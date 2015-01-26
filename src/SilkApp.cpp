#include <QWidget>

#include "SilkApp.h"
#include "TabViewGroup.h"
#include "DocumentService.h"
#include "TextEditView.h"
#include "TabView.h"
#include "MainWindow.h"

namespace {
template <typename T>
T findParent(QWidget* widget) {
  if (!widget)
    return nullptr;

  T desiredWidget = qobject_cast<T>(widget->parentWidget());
  if (desiredWidget)
    return desiredWidget;
  return findParent<T>(widget->parentWidget());
}
}

TabBar* SilkApp::tabBarAt(int x, int y) {
  foreach(MainWindow * window, MainWindow::windows()) {
    if (TabBar* tabBar = window->tabViewGroup()->tabBarAt(x, y)) {
      return tabBar;
    }
  }

  return nullptr;
}

SilkApp::SilkApp(int& argc, char** argv) : QApplication(argc, argv) {
  // Track active TabView
  QObject::connect(this, &QApplication::focusChanged, [this](QWidget*, QWidget* focusedWidget) {
    qDebug("focusChanged");
    if (TextEditView* editView = qobject_cast<TextEditView*>(focusedWidget)) {
      if (TabView* tabView = findParent<TabView*>(editView)) {
        if (TabViewGroup* tabViewGroup = findParent<TabViewGroup*>(tabView)) {
          tabViewGroup->setActiveTab(tabView);
        } else {
          qDebug("unable to find the parent TabViewGroup");
        }
      } else {
        qDebug("can't find TabView in ancestor");
      }
    } else {
      qDebug("focused widget is not TextEditView");
    }
  });
}

bool SilkApp::event(QEvent* event) {
  switch (event->type()) {
    case QEvent::FileOpen:
      qDebug("FileOpen event");
      DocumentService::open(static_cast<QFileOpenEvent*>(event)->file());
      return true;
    default:
      return QApplication::event(event);
  }
}
