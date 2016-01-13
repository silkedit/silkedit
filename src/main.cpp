#include <QStringList>
#include <QTime>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTimer>

#include "SilkApp.h"
#include "TabView.h"
#include "Window.h"
#include "KeymapManager.h"
#include "CommandManager.h"
#include "DocumentManager.h"
#include "TextEditView.h"
#include "PlatformUtil.h"
#include "TestUtil.h"
#include "Helper.h"
#include "MenuBar.h"
#include "MetaTypeInitializer.h"
#include "QObjectHelper.h"
#include "API.h"
#include "core/ConditionManager.h"
#include "core/PackageManager.h"
#include "core/Config.h"
#include "core/ThemeManager.h"
#include "breakpad/crash_handler.h"

using core::PackageManager;
using core::Config;
using core::ConditionManager;
using core::ThemeManager;

int main(int argv, char** args) {
  QTime startTime = QTime::currentTime();
  PlatformUtil::enableMnemonicOnMac();
  SilkApp app(argv, args);
#ifdef QT_NO_DEBUG
  // crash dumps output location setting.
  Breakpad::CrashHandler::instance()->Init(QDir::tempPath());
#endif

  // call a bunch of qRegisterMetaType calls
  MetaTypeInitializer::init();

  // instantiate singleton objects to set their thrad affinity to current thread
  QObjectHelper::singleton();
  API::singleton();

  ConditionManager::init();

  PackageManager::loadGrammers();

  ThemeManager::load();

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

  // As a special case, a QTimer with a timeout of 0 will time out as soon as all the events in the
  // window system's event queue have been processe
  QTimer::singleShot(0, &Helper::singleton(), &Helper::init);

  QStringList arguments = app.arguments();
  if (arguments.size() > 1) {
    DocumentManager::open(arguments.at(1));
  }

  //  new TestUtil();

  int passed = startTime.msecsTo(QTime::currentTime());
  qDebug("startup time: %d [ms]", passed);
  return app.exec();
}
