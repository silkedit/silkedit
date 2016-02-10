#include <QWidget>
#include <QTranslator>
#include <QChildEvent>
#include <QProcess>
#include <QDebug>

#include "App.h"
#include "TabViewGroup.h"
#include "DocumentManager.h"
#include "TextEditView.h"
#include "TabView.h"
#include "Window.h"
#include "version.h"
#include "SilkStyle.h"
#include "KeymapManager.h"
#include "core/ObjectStore.h"
#include "core/Constants.h"

using core::Constants;
using core::ObjectStore;

App* App::s_app = nullptr;

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

TabBar* App::tabBarAt(int x, int y) {
  foreach (Window* window, Window::windows()) {
    if (TabBar* tabBar = window->tabViewGroup()->tabBarAt(x, y)) {
      return tabBar;
    }
  }

  return nullptr;
}

App::App(int& argc, char** argv)
    : QApplication(argc, argv), m_translator(nullptr), m_qtTranslator(nullptr) {
  setApplicationVersion(VERSION);
  setStyle(new SilkStyle());
  setAttribute(Qt::AA_UseHighDpiPixmaps);
  s_app = this;

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
    if (TabView* tabView = findParent<TabView*>(focusedWidget)) {
      if (TabViewGroup* tabViewGroup = findParent<TabViewGroup*>(tabView)) {
        tabViewGroup->setActiveTab(tabView);
      }
    }
  });

  installEventFilter(this);
}

bool App::event(QEvent* event) {
  switch (event->type()) {
    case QEvent::FileOpen:
      qDebug("FileOpen event");
      DocumentManager::singleton().open(static_cast<QFileOpenEvent*>(event)->file());
      return true;
    default:
      return QApplication::event(event);
  }
}

bool App::eventFilter(QObject*, QEvent* event) {
  switch (event->type()) {
    case QEvent::ChildAdded: {
      QObject* child = static_cast<QChildEvent*>(event)->child();
      if (child && child->property(OBJECT_STATE).isValid()) {
        qDebug() << "set" << OBJECT_STATE << "of" << child->metaObject()->className() << "to"
                 << ObjectStore::ObjectState::NewFromJSButHasParent;
        child->setProperty(OBJECT_STATE,
                           QVariant::fromValue(ObjectStore::ObjectState::NewFromJSButHasParent));
        Q_ASSERT(child->property(OBJECT_STATE).isValid());
        Q_ASSERT(child->property(OBJECT_STATE).value<ObjectStore::ObjectState>() ==
                 ObjectStore::ObjectState::NewFromJSButHasParent);
      }
      break;
    }
    // Qt doc says 'child might have already been destructed'. Is the code below safe?
    case QEvent::ChildRemoved: {
      QObject* child = static_cast<QChildEvent*>(event)->child();
      if (child && child->property(OBJECT_STATE).isValid()) {
        qDebug() << "set" << OBJECT_STATE << "of" << child->metaObject()->className() << "to"
                 << ObjectStore::ObjectState::NewFromJS;
        child->setProperty(OBJECT_STATE, QVariant::fromValue(ObjectStore::ObjectState::NewFromJS));
        Q_ASSERT(child->property(OBJECT_STATE).isValid());
        Q_ASSERT(child->property(OBJECT_STATE).value<ObjectStore::ObjectState>() ==
                 ObjectStore::ObjectState::NewFromJS);
      }
      break;
    }
    case QEvent::KeyPress: {
      auto keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent && KeymapManager::singleton().handle(keyEvent)) {
        keyEvent->accept();
        return true;
      }
      break;
    }
    // If we don't intercept ShortcutOverride event, KeyPress event doesn't come here
    // https://bugreports.qt.io/browse/QTBUG-30164
    case QEvent::ShortcutOverride: {
      event->accept();
      return true;
    }
    default:
      return false;
  }
  return false;
}

void App::setupTranslator(const QString& locale) {
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

TextEditView* App::activeTextEditView() {
  TabView* tabView = activeTabView();
  if (tabView) {
    return qobject_cast<TextEditView*>(tabView->activeView());
  } else {
    qDebug("active tab view is null");
    return nullptr;
  }
}

TabView* App::activeTabView() {
  TabViewGroup* tabViewGroup = activeTabViewGroup();
  if (tabViewGroup) {
    return tabViewGroup->activeTab();
  } else {
    qDebug("active tab view group is null");
    return nullptr;
  }
}

TabViewGroup* App::activeTabViewGroup() {
  Window* window = activeWindow();
  if (window) {
    return window->tabViewGroup();
  } else {
    qDebug("active window is null");
    return nullptr;
  }
}

Window* App::activeWindow() {
  return qobject_cast<Window*>(QApplication::activeWindow());
}

void App::setActiveWindow(QWidget* act) {
  QApplication::setActiveWindow(act);
}

void App::restart() {
  if (s_app) {
    QProcess::startDetached(QApplication::applicationFilePath());
    s_app->exit();
  }
}
