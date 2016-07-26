#include <algorithm>
#include <QWidget>
#include <QTranslator>
#include <QChildEvent>
#include <QProcess>
#include <QDebug>
#include <QFontDatabase>

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
#include "core/Util.h"

using core::Constants;
using core::ObjectStore;
using core::SyntaxHighlighterThread;
using core::Util;

App* App::s_app = nullptr;
bool App::m_isCleanedUp = false;

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
    : QtSingleApplication(argc, argv),
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
  // Install Source Han Code JP fonts
  installFont(":/SourceHanCodeJP-Normal.otf");
  installFont(":/SourceHanCodeJP-Regular.otf");
  installFont(":/SourceHanCodeJP-Bold.otf");
#endif

  setStyleSheet(Util::readResource(":/stylesheets/stylesheet.css"));

  // Track active TabView
  connect(this, &QApplication::focusChanged, [this](QWidget*, QWidget* focusedWidget) {
    //    qDebug("focusChanged");
    if (TabView* tabView = findParent<TabView*>(focusedWidget)) {
      if (TabViewGroup* tabViewGroup = findParent<TabViewGroup*>(tabView)) {
        tabViewGroup->setActiveTabView(tabView);
      }
    }

    if (auto window = findActiveWindow()) {
      window->updateTitle();
      setActivationWindow(window);
    }
  });

  connect(this, &QApplication::lastWindowClosed, [this]{
    setActivationWindow(nullptr);
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
      cleanup();
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

  if (m_isCleanedUp) {
    return;
  }

  m_isQuitting = true;

  App::saveSession();

  Helper::singleton().deactivatePackages();

  for (auto window : Window::windows()) {
    if (!window->close()) {
      return;
    }
  }

  // emit destroyed signal to JS side before shutting down Node
  ObjectStore::clearAssociatedJSObjects();

  SyntaxHighlighterThread::singleton().quit();

  Helper::singleton().cleanup();

  m_isCleanedUp = true;
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

  // Load qt_<locale>.qm to translate Qt's builtin messages (dialog messages, context menu, etc.)
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
}

TabView* App::getActiveTabViewOrCreate() {
  TabViewGroup* tabViewGroup = activeTabViewGroup();
  if (tabViewGroup) {
    if (tabViewGroup->activeTabView()) {
      return tabViewGroup->activeTabView();
    } else {
      return tabViewGroup->addNewTabView();
    }
  }

  return nullptr;
}

void App::setDefaultFont(QString locale) {
  // change default UI font based on locale
  QFontDatabase database;
  if (locale == "ja" || locale == "ja_JP") {
#ifdef Q_OS_WIN
    QList<std::tuple<QString, int>> fontInfos = {
        // Yu Gothic UI for Windows 10
        std::make_tuple(QStringLiteral("Yu Gothic UI"), 10),
        // Meiryo UI for Windows 7&8
        std::make_tuple(QStringLiteral("Meiryo UI"), 10)};
#elif defined Q_OS_MAC
    QList<std::tuple<QString, int>> fontInfos = {};
#endif

    for (const auto& fontInfo : fontInfos) {
      if (database.hasFamily(std::get<0>(fontInfo))) {
        setFont(QFont(std::get<0>(fontInfo), std::get<1>(fontInfo)));
        break;
      }
    }
  }
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
    return tabViewGroup->activeTabView();
  } else {
    qDebug("active tab view group is null");
    return nullptr;
  }
}

TabViewGroup* App::activeTabViewGroup() {
  if (auto window = activeWindow()) {
    return window->tabViewGroup();
  } else {
    qDebug("active window is null");
    return nullptr;
  }
}

Window* App::findActiveWindow() {
  // Try to find the window that has the input focus
  Window* window = qobject_cast<Window*>(QApplication::activeWindow());

  // If we can't find it, try to find the top level window
  if (!window && !QApplication::topLevelWidgets().isEmpty()) {
    auto widgets = QApplication::topLevelWidgets();
    auto it = std::find_if(widgets.constBegin(), widgets.constEnd(),
                           [](QWidget* widget) { return qobject_cast<Window*>(widget); });
    if (it != widgets.constEnd()) {
      window = qobject_cast<Window*>(*it);
      Q_ASSERT(window);
    }
  }

  return window;
}

Window* App::activeWindow() {
  return activationWindow() ? qobject_cast<Window*>(activationWindow()) : findActiveWindow();
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
  QSettings settings(Constants::singleton().sessionPath(), QSettings::IniFormat);
  settings.clear();
  Window::saveWindowsState(s_app->activeWindow(), settings);
}

void App::loadSession() {
  QSettings settings(Constants::singleton().sessionPath(), QSettings::IniFormat);
  Window::loadWindowsState(settings);
}
