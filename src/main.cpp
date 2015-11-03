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
#include "PluginManager.h"
#include "Context.h"
#include "MenuBar.h"
#include "core/PackageManager.h"
#include "core/Config.h"

using core::PackageManager;
using core::Config;

int main(int argv, char** args) {
  QTime startTime = QTime::currentTime();
  PlatformUtil::enableMnemonicOnMac();

  SilkApp app(argv, args);

  Context::init();

  PackageManager::loadPackages();

  Config::singleton().init();

  // Setup translator after initializing ConfigModel
  app.setupTranslator(Config::singleton().locale());

  // Load keymap settings after registering commands
  KeymapManager::singleton().load();

  // Create default menu bar before creating any new window
  MenuBar::init();

  Window* w = Window::createWithNewFile();
  w->show();

  //   Set focus to active edit view
  if (auto v = w->activeTabView()->activeEditView()) {
    v->setFocus();
  }

  API::init();
  PluginManager::singleton().init();

  QStringList arguments = app.arguments();
  if (arguments.size() > 1) {
    DocumentManager::open(arguments.at(1));
  }

  //  new TestUtil();

  int passed = startTime.msecsTo(QTime::currentTime());
  qDebug("startup time: %d [ms]", passed);
  return app.exec();
}
