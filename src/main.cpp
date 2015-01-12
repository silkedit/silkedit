#include <QStringList>

#include "PackageService.h"
#include "SilkApp.h"
#include "STabWidget.h"
#include "MainWindow.h"
#include "KeymapService.h"
#include "ConfigService.h"
#include "CommandService.h"
#include "DocumentService.h"
#include "MenuBar.h"
#include "commands/ToggleVimEmulationCommand.h"
#include "Session.h"

int main(int argv, char** args) {
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
  if (auto v = w->activeTabWidget()->activeEditView()) {
    v->setFocus();
  }

  if (ConfigService::isTrue("enable_vim_emulation")) {
    viEngine.enable();
  }

  MenuBar menuBar(nullptr);

  Session::singleton().init();

  QStringList arguments = app.arguments();
  if (arguments.size() > 1) {
    DocumentService::open(arguments.at(1));
  }

  //  new Dummy();

  return app.exec();
}
