#include <QWidget>

#include "SilkApp.h"
#include "TextEditView.h"
#include "MainWindow.h"
#include "STabWidget.h"
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
  // Track active TextEditView and STabWidget
  QObject::connect(this, &QApplication::focusChanged, [this](QWidget*, QWidget* now) {
    qDebug("focusChanged");
    if (TextEditView* editView = qobject_cast<TextEditView*>(now)) {
      if (STabWidget* tabWidget = findParent<STabWidget*>(editView)) {
        if (MainWindow* window = qobject_cast<MainWindow*>(tabWidget->window())) {
          window->setActiveTabWidget(tabWidget);
        } else {
          qDebug("top window is not MainWindow");
        }
      } else {
        qDebug("can't find STabWidget in ancestor");
      }
    } else {
      qDebug("now is not TextEditView");
    }
  });

  setWindowIcon(QIcon(":/app_icon_064.png"));
}

bool SilkApp::event(QEvent* event) {
  switch (event->type()) {
    case QEvent::FileOpen:
      qDebug("FileOpen event");
      DocumentService::singleton().open(static_cast<QFileOpenEvent*>(event)->file());
      return true;
    default:
      return QApplication::event(event);
  }
}
