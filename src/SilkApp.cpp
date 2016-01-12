#include <QWidget>
#include <QTranslator>
#include <QChildEvent>
#include <QProcess>

#include "SilkApp.h"
#include "TabViewGroup.h"
#include "DocumentManager.h"
#include "TextEditView.h"
#include "TabView.h"
#include "Window.h"
#include "Helper.h"
#include "version.h"
#include "SilkStyle.h"
#include "API.h"
#include "ObjectStore.h"
#include "core/Constants.h"

using core::Constants;

SilkApp* SilkApp::s_app = nullptr;

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

#ifdef Q_OS_WIN
int installFont(const QString& path) {
  auto result = QFontDatabase::addApplicationFont(path);
  if (result == -1) {
    qWarning("Failed to install %s", qPrintable(path));
  }
  return result;
}
#endif
}

TabBar* SilkApp::tabBarAt(int x, int y) {
  foreach (Window* window, Window::windows()) {
    if (TabBar* tabBar = window->tabViewGroup()->tabBarAt(x, y)) {
      return tabBar;
    }
  }

  return nullptr;
}

SilkApp::SilkApp(int& argc, char** argv)
    : QApplication(argc, argv), m_translator(nullptr), m_qtTranslator(nullptr) {
  setApplicationVersion(VERSION);
  setStyle(new SilkStyle());
  setAttribute(Qt::AA_UseHighDpiPixmaps);
  s_app = this;

  connect(this, &QApplication::aboutToQuit, &Helper::singleton(), &Helper::cleanup);

#ifdef Q_OS_WIN
// application font doesn't work with DirectWrite font engine
// https://bugreports.qt.io/browse/QTBUG-18711
// Install Source Han Code JP fonts
//  installFont(":/SourceHanCodeJP-Normal.otf");
//  installFont(":/SourceHanCodeJP-Regular.otf");
//  installFont(":/SourceHanCodeJP-Bold.otf");
#endif

  QFile file(":/stylesheet.css");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setStyleSheet(file.readAll());
    file.close();
  }

  // Track active TabView
  QObject::connect(this, &QApplication::focusChanged, [this](QWidget*, QWidget* focusedWidget) {
    //    qDebug("focusChanged");
    if (TextEditView* editView = qobject_cast<TextEditView*>(focusedWidget)) {
      if (TabView* tabView = findParent<TabView*>(editView)) {
        if (TabViewGroup* tabViewGroup = findParent<TabViewGroup*>(tabView)) {
          tabViewGroup->setActiveTab(tabView);

          // send focusChangedEvent to silkedit_helper
          Helper::singleton().sendFocusChangedEvent("TextEditView");
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

  installEventFilter(this);
}

bool SilkApp::event(QEvent* event) {
  switch (event->type()) {
    case QEvent::FileOpen:
      qDebug("FileOpen event");
      DocumentManager::open(static_cast<QFileOpenEvent*>(event)->file());
      return true;
    default:
      return QApplication::event(event);
  }
}

bool SilkApp::eventFilter(QObject*, QEvent* event) {
  switch (event->type()) {
    case QEvent::ChildAdded: {
      QObject* child = static_cast<QChildEvent*>(event)->child();
      if (child && child->property(OBJECT_STATE).isValid()) {
        qDebug() << "set" << OBJECT_STATE << "of" << child->metaObject()->className()
                 << "to" << ObjectStore::ObjectState::NewFromJSButHasParent;
        child->setProperty(OBJECT_STATE,
                           ObjectStore::ObjectState::NewFromJSButHasParent);
      }
      break;
    }
    // Qt doc says 'child might have already been destructed'. Is the code below safe?
    case QEvent::ChildRemoved: {
      QObject* child = static_cast<QChildEvent*>(event)->child();
      if (child && child->property(OBJECT_STATE).isValid()) {
        qDebug() << "set" << OBJECT_STATE << "of" << child->metaObject()->className()
                 << "to" << ObjectStore::ObjectState::NewFromJS;
        child->setProperty(OBJECT_STATE, ObjectStore::ObjectState::NewFromJS);
      }
      break;
    }
    default:
      return false;
  }
  return false;
}

void SilkApp::setupTranslator(const QString& locale) {
  if (m_translator) {
    m_translator->deleteLater();
    removeTranslator(m_translator);
  }

  m_translator = new QTranslator(this);
  // Load silkedit_<locale>.qm to translate SilkEdit menu
  bool result =
      m_translator->load("silkedit_" + locale, Constants::singleton().translationDirPath());
  if (!result) {
    qWarning() << "Failed to load" << qPrintable("silkedit_");
  }
  installTranslator(m_translator);

#ifdef Q_OS_MAC
  // Load qt_<locale>.qm to translate Mac application menu
  if (m_qtTranslator) {
    m_qtTranslator->deleteLater();
    removeTranslator(m_qtTranslator);
  }

  m_qtTranslator = new QTranslator(this);
  result = m_qtTranslator->load("qt_" + locale, Constants::singleton().translationDirPath());
  if (!result) {
    qWarning() << "Failed to load" << qPrintable("qt_");
  }
  installTranslator(m_qtTranslator);
#endif
}

TextEditView* SilkApp::activeTextEditView() {
  TabView* tabView = activeTabView();
  if (tabView) {
    return tabView->activeEditView();
  } else {
    qDebug("active tab view is null");
    return nullptr;
  }
}

TabView* SilkApp::activeTabView() {
  TabViewGroup* tabViewGroup = activeTabViewGroup();
  if (tabViewGroup) {
    return tabViewGroup->activeTab();
  } else {
    qDebug("active tab view group is null");
    return nullptr;
  }
}

TabViewGroup* SilkApp::activeTabViewGroup() {
  Window* window = activeWindow();
  if (window) {
    return window->tabViewGroup();
  } else {
    qDebug("active window is null");
    return nullptr;
  }
}

Window* SilkApp::activeWindow() {
  return qobject_cast<Window*>(QApplication::activeWindow());
}

void SilkApp::restart() {
  if (s_app) {
    QProcess::startDetached(QApplication::applicationFilePath());
    s_app->exit();
  }
}
