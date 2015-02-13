#include <QStringList>

#include "PackageService.h"
#include "SilkApp.h"
#include "TabView.h"
#include "MainWindow.h"
#include "KeymapService.h"
#include "ConfigService.h"
#include "CommandService.h"
#include "DocumentService.h"
#include "commands/ToggleVimEmulationCommand.h"
#include "Session.h"
#include "ViEngine.h"
#include "TextEditView.h"
#include "PlatformUtil.h"
#include "TestUtil.h"
#include "plugin_service/PluginService.h"
#include "MenuService.h"

int main(int argv, char** args) {
  PlatformUtil::enableMnemonicOnMac();

  SilkApp app(argv, args);

  PackageService::loadPackages();

  ConfigService::load();

  CommandService::init();

  ViEngine viEngine;

  std::unique_ptr<ToggleVimEmulationCommand> toggleVimEmulationCmd(
      new ToggleVimEmulationCommand(&viEngine));
  CommandService::add(std::move(toggleVimEmulationCmd));

  //   Load keymap settings after registering commands
  KeymapService::singleton().load();

  MainWindow* w = MainWindow::createWithNewFile();
  w->show();

  //   Set focus to active edit view
  if (auto v = w->activeTabView()->activeEditView()) {
    v->setFocus();
  }

  if (ConfigService::isTrue("enable_vim_emulation")) {
    viEngine.enable();
  }

  MenuService::init();

  Session::singleton().init();

  API::init();
  PluginService::singleton().init();

  QStringList arguments = app.arguments();
  if (arguments.size() > 1) {
    DocumentService::open(arguments.at(1));
  }

  //  new TestUtil();

  return app.exec();
}
