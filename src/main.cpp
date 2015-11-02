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
#include "core/Constants.h"

using core::Constants;

using core::ConfigModel;

int main(int argv, char** args) {
  QTime startTime = QTime::currentTime();
  PlatformUtil::enableMnemonicOnMac();

  SilkApp app(argv, args);

  Context::init();

  core::PackageManager::loadPackages();

  ConfigModel::load();

  // Populate session values after loading configs
  core::Session::singleton().init();

  // Load keymap settings after registering commands
  KeymapManager::singleton().load();

  // setup translators
  QTranslator translator;
  QTranslator qtTranslator;
  // Load silkedit_<locale>.qm to translate SilkEdit menu
  bool result =
      translator.load("silkedit_" + ConfigModel::locale(), Constants::translationDirPath());
  if (!result) {
    qWarning() << "Failed to load" << qPrintable("silkedit_");
  }

  // Load qt_<locale>.qm to translate Mac application menu
  result = qtTranslator.load("qt_" + ConfigModel::locale(), Constants::translationDirPath());
  if (!result) {
    qWarning() << "Failed to load" << qPrintable("qt_");
  }
  app.installTranslator(&translator);
  app.installTranslator(&qtTranslator);

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
