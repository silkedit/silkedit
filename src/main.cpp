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
#include "commands/ToggleVimEmulationCommand.h"
#include "Session.h"
#include "ViEngine.h"
#include "TextEditView.h"
#include "PlatformUtil.h"
#include "TestUtil.h"
#include "PluginManager.h"
#include "Context.h"
#include "msgpackHelper.h"

int main(int argv, char** args) {
  QTime startTime = QTime::currentTime();
  PlatformUtil::enableMnemonicOnMac();

  SilkApp app(argv, args);

  // register object_with_zone to make it avaialble for queued signal-slot communication
  qRegisterMetaType<object_with_zone>();

  Context::init();

  PackageManager::loadPackages();

  ConfigManager::load();

//  ViEngine viEngine;

  //   Load keymap settings after registering commands
  KeymapManager::singleton().load();

  Window* w = Window::createWithNewFile();
  w->show();

  //   Set focus to active edit view
  if (auto v = w->activeTabView()->activeEditView()) {
    v->setFocus();
  }

  // todo: replace with js version
//  if (ConfigManager::isTrue("enable_vim_emulation")) {
//    viEngine.enable();
//  }

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
