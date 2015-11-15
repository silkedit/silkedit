#include <QStringList>
#include <QTime>
#include <QTranslator>
#include <QLibraryInfo>

#include "SilkApp.h"
#include "TabView.h"
#include "Window.h"
#include "KeymapManager.h"
#include "CommandManager.h"
#include "DocumentManager.h"
#include "TextEditView.h"
#include "PlatformUtil.h"
#include "TestUtil.h"
#include "HelperProxy.h"
#include "core/ConditionManager.h"
#include "MenuBar.h"
#include "core/PackageManager.h"
#include "core/Config.h"
#include "breakpad/crash_handler.h"

using core::PackageManager;
using core::Config;
using core::ConditionManager;

int main(int argv, char** args) {
  QTime startTime = QTime::currentTime();
  PlatformUtil::enableMnemonicOnMac();
#ifdef QT_NO_DEBUG
  // crash dumps output location setting.
  Breakpad::CrashHandler::instance()->Init(QDir::tempPath());
#endif
  SilkApp app(argv, args);

  ConditionManager::init();

  PackageManager::loadPackages();

  Config::singleton().init();

  // Setup translator after initializing Config
  app.setupTranslator(Config::singleton().locale());

  // Load keymap settings after registering commands
  KeymapManager::singleton().loadUserKeymap();

  // Create default menu bar before creating any new window
  MenuBar::init();

  Window* w = Window::createWithNewFile();
  w->show();

  //   Set focus to active edit view
  if (auto v = w->activeTabView()->activeEditView()) {
    v->setFocus();
  }

  API::init();
  HelperProxy::singleton().init();

  QStringList arguments = app.arguments();
  if (arguments.size() > 1) {
    DocumentManager::open(arguments.at(1));
  }

  //  new TestUtil();

  int passed = startTime.msecsTo(QTime::currentTime());
  qDebug("startup time: %d [ms]", passed);
  return app.exec();
}
