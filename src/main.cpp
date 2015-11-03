#include <QStringList>
#include <QTime>
#include <QTranslator>
#include <QLibraryInfo>

#include "core/PackageManager.h"
#include "SilkApp.h"
#include "TabView.h"
#include "Window.h"
#include "KeymapManager.h"
#include "core/ConfigModel.h"
#include "CommandManager.h"
#include "DocumentManager.h"
#include "core/Session.h"
#include "TextEditView.h"
#include "PlatformUtil.h"
#include "TestUtil.h"
#include "PluginManager.h"
#include "Context.h"
#include "MenuBar.h"

using core::ConfigModel;

int main(int argv, char** args) {
  QTime startTime = QTime::currentTime();
  PlatformUtil::enableMnemonicOnMac();

  SilkApp app(argv, args);

  Context::init();

  core::PackageManager::loadPackages();

  ConfigModel::load();

  // Setup translator after initializing ConfigModel
  app.setupTranslator(ConfigModel::locale());

  // Populate session values after loading configs
  core::Session::singleton().init();

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
