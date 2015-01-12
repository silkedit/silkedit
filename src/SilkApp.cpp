#include <QWidget>

#include "SilkApp.h"
#include "TextEditView.h"
#include "MainWindow.h"
#include "TabWidget.h"
#include "DocumentService.h"

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

SilkApp::SilkApp(int& argc, char** argv) : QApplication(argc, argv) {
  // Track active TextEditView and TabWidget
  QObject::connect(this, &QApplication::focusChanged, [this](QWidget*, QWidget* now) {
    qDebug("focusChanged");
    if (TextEditView* editView = qobject_cast<TextEditView*>(now)) {
      if (TabWidget* tabWidget = findParent<TabWidget*>(editView)) {
        if (MainWindow* window = qobject_cast<MainWindow*>(tabWidget->window())) {
          window->setActiveTabWidget(tabWidget);
        } else {
          qDebug("top window is not MainWindow");
        }
      } else {
        qDebug("can't find TabWidget in ancestor");
      }
    } else {
      qDebug("now is not TextEditView");
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
