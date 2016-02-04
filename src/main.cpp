#include <QStringList>
#include <QTime>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTimer>
#include <QDir>

#include "App.h"
#include "TabView.h"
#include "Window.h"
#include "KeymapManager.h"
#include "CommandManager.h"
#include "DocumentManager.h"
#include "TextEditView.h"
#include "PlatformUtil.h"
#include "Helper.h"
#include "MenuBar.h"
#include "MetaTypeInitializer.h"
#include "core/Condition.h"
#include "core/PackageManager.h"
#include "core/Config.h"
#include "core/ThemeManager.h"
#include "core/Util.h"
#include "core/Constants.h"
#include "breakpad/crash_handler.h"
#include "node_main.h"

using core::PackageManager;
using core::Config;
using core::Condition;
using core::ThemeManager;
using core::Util;
using core::Constants;

int main(int argc, char** argv) {
  QTime startTime = QTime::currentTime();
  PlatformUtil::enableMnemonicOnMac();
  App app(argc, argv);
#ifdef QT_NO_DEBUG
  // crash dumps output location setting.
  Breakpad::CrashHandler::instance()->Init(QDir::tempPath());
#endif

  QStringList arguments = app.arguments();

  // Run SilkEdit as normal Node.js
  if (arguments.contains(Constants::RUN_AS_NODE)) {
    arguments.removeOne(Constants::RUN_AS_NODE);
    return nodeMain(arguments.size(), Util::toArgv(arguments));
  }

  // call a bunch of qRegisterMetaType calls
  MetaTypeInitializer::init();

  Condition::init();

  PackageManager::loadGrammers();

  ThemeManager::load();

  Config::singleton().init();

  // Setup translator after initializing Config
  app.setupTranslator(Config::singleton().locale());

  // Load keymap settings after registering commands
  KeymapManager::singleton().loadUserKeymap();

  // Create default menu bar before creating any new window
  MenuBar::init();

  Window* window = Window::createWithNewFile();
  QObject::connect(window, &Window::firstPaintEventFired, [&] {
    qDebug() << "firstPaintEventFired";
    // Start Node.js event loop after showing the first window
    // As a special case, a QTimer with a timeout of 0 will time out as soon as all the events in
    // the window system's event queue have been processed
    QTimer::singleShot(0, &Helper::singleton(), &Helper::init);
  });
  window->show();

  //   Set focus to active view
  if (auto v = window->activeTabView()->activeView()) {
    v->setFocus();
  }

  if (arguments.size() > 1) {
    DocumentManager::singleton().open(arguments.at(1));
  }

  int passed = startTime.msecsTo(QTime::currentTime());
  qDebug("startup time: %d [ms]", passed);
  return app.exec();
}
