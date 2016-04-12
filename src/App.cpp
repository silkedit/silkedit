#include <QWidget>
#include <QTranslator>
#include <QChildEvent>
#include <QProcess>
#include <QDebug>

#include "App.h"
#include "TabViewGroup.h"
#include "DocumentManager.h"
#include "TextEdit.h"
#include "TabView.h"
#include "Window.h"
#include "version.h"
#include "SilkStyle.h"
#include "KeymapManager.h"
#include "Helper.h"
#include "core/ObjectStore.h"
#include "core/Constants.h"
#include "core/SyntaxHighlighter.h"

using core::Constants;
using core::ObjectStore;
using core::SyntaxHighlighterThread;

App* App::s_app = nullptr;
bool App::m_isSessionSaved = false;

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
    : QApplication(argc, argv),
      m_translator(nullptr),
      m_qtTranslator(nullptr),
      m_isQuitting(false) {
  setApplicationVersion(VERSION);
  setStyle(new SilkStyle());
  setAttribute(Qt::AA_UseHighDpiPixmaps);
#ifdef Q_OS_MAC
  setQuitOnLastWindowClosed(false);
#endif
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
  connect(this, &QApplication::focusChanged, [this](QWidget*, QWidget* focusedWidget) {
    //    qDebug("focusChanged");
    if (TabView* tabView = findParent<TabView*>(focusedWidget)) {
      if (TabViewGroup* tabViewGroup = findParent<TabViewGroup*>(tabView)) {
        tabViewGroup->setActiveTab(tabView);
      }
    }

    if (auto window = activeWindow()) {
      window->updateTitle();
    }
  });

  connect(this, &App::aboutToQuit, this, &App::cleanup);
  connect(this, &App::commitDataRequest, this, &App::cleanup);
}

bool App::event(QEvent* event) {
  switch (event->type()) {
    case QEvent::FileOpen:
      qDebug("FileOpen event");
      return DocumentManager::singleton().open(static_cast<QFileOpenEvent*>(event)->file()) != -1;
    // QCloseEvent comes when logout (in this case, all windows are closed before aboutToQuit is
    // emitted)
    case QEvent::Close:
      saveSession();
      break;
    default:
      return QApplication::event(event);
  }

  return QApplication::event(event);
}

bool App::notify(QObject* receiver, QEvent* event) {
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
      // KeyPress event is sent multiple times to each different receivers.
      // We hadle key press event only when receiver is window type
      if (receiver->isWindowType()) {
        auto keyEvent = static_cast<QKeyEvent*>(event);
        qDebug() << keyEvent;
        if (keyEvent && KeymapManager::singleton().handle(keyEvent)) {
          keyEvent->accept();
          return true;
        }
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
      break;
  }

  return QApplication::notify(receiver, event);
}

void App::cleanup() {
  qDebug() << "cleanup";
  m_isQuitting = true;

  App::saveSession();

  Helper::singleton().deactivatePackages();

  for (auto window : Window::windows()) {
    if (!window->close()) {
      return;
    }
  }

  SyntaxHighlighterThread::singleton().quit();
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

TabView* App::getActiveTabViewOrCreate() {
  TabViewGroup* tabViewGroup = activeTabViewGroup();
  if (tabViewGroup) {
    if (tabViewGroup->activeTab()) {
      return tabViewGroup->activeTab();
    } else {
      return tabViewGroup->addNewTabView();
    }
  }

  return nullptr;
}

TextEdit* App::activeTextEdit() {
  TabView* tabView = activeTabView();
  if (tabView) {
    return qobject_cast<TextEdit*>(tabView->activeView());
  } else {
    return qobject_cast<TextEdit*>(focusWidget());
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
  // Try to find the window that has the input focus
  Window* window = qobject_cast<Window*>(QApplication::activeWindow());

  // If we can't find it, try to find the top level window
  if (!window && !QApplication::topLevelWidgets().isEmpty()) {
    window = qobject_cast<Window*>(QApplication::topLevelWidgets().first());
  }

  return window;
}

void App::setActiveWindow(QWidget* act) {
  QApplication::setActiveWindow(act);
}

QWidget* App::focusWidget() {
  return QApplication::focusWidget();
}

QWidget* App::activePopupWidget() {
  return QApplication::activePopupWidget();
}

void App::postEvent(QObject* receiver, QEvent* event, int priority) {
  QApplication::postEvent(receiver, event, priority);
}

void App::restart() {
  if (s_app) {
    QProcess::startDetached(QApplication::applicationFilePath());
    s_app->exit();
  }
}

void App::saveSession() {
  if (m_isSessionSaved) {
    return;
  }

  QSettings settings(Constants::singleton().sessionPath(), QSettings::IniFormat);
  settings.clear();
  Window::saveWindowsState(s_app->activeWindow(), settings);
  m_isSessionSaved = true;
}

void App::loadSession() {
  QSettings settings(Constants::singleton().sessionPath(), QSettings::IniFormat);
  Window::loadWindowsState(settings);
}
