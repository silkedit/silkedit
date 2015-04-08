#include <QStringList>
#include <QTime>

#include "PackageManager.h"
#include "SilkApp.h"
#include "TabView.h"
#include "Window.h"
#include "KeymapManager.h"
#include "ConfigManager.h"
#include "CommandManager.h"
#include "DocumentManager.h"
#include "Session.h"
#include "TextEditView.h"
#include "PlatformUtil.h"
#include "TestUtil.h"
#include "PluginManager.h"
#include "Context.h"
#include "MenuBar.h"

int main(int argv, char** args) {
  QTime startTime = QTime::currentTime();
  PlatformUtil::enableMnemonicOnMac();

  SilkApp app(argv, args);

  Context::init();

  PackageManager::loadPackages();

  ConfigManager::load();

  //   Load keymap settings after registering commands
  KeymapManager::singleton().load();

  // Create default menu bar before creating any new window
  MenuBar::init();

  Window* w = Window::createWithNewFile();
  w->show();

  //   Set focus to active edit view
  if (auto v = w->activeTabView()->activeEditView()) {
    v->setFocus();
  }

  Session::singleton().init();

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
