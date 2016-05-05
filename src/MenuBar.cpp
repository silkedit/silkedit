#include <QAction>
#include <QMessageBox>

#include "MenuBar.h"
#include "CommandAction.h"
#include "OpenRecentItemManager.h"
#include "App.h"
#include "CommandManager.h"
#include "version.h"
#include "ConfigDialog.h"
#include "AboutDialog.h"
#include "Window.h"
#include "commands/ReopenLastClosedFileCommand.h"
#include "commands/PackageCommand.h"
#include "core/ThemeManager.h"
#include "core/Config.h"
#include "core/PackageMenu.h"

using core::ThemeManager;
using core::Theme;
using core::Config;
using core::PackageMenu;

MenuBar* MenuBar::s_globalMenuBar;

void MenuBar::init() {
  s_globalMenuBar = new MenuBar();
}

MenuBar::MenuBar(QWidget* parent) : QMenuBar(parent) {
  // File Menu
  const QString& fileMenuStr = Config::singleton().enableMnemonic() ? tr("&File") : tr("File");
  auto fileMenu = new PackageMenu(fileMenuStr);
  addMenu(fileMenu);
  fileMenu->addMenu(OpenRecentItemManager::singleton().openRecentMenu());
  fileMenu->setObjectName("file");

  const QString& exitMenuStr = Config::singleton().enableMnemonic() ? tr("&Exit") : tr("Exit");
  QAction* exitAction = new QAction(exitMenuStr, fileMenu);
  exitAction->setObjectName(QStringLiteral("exit"));
  exitAction->setMenuRole(QAction::QuitRole);
  connect(exitAction, &QAction::triggered, App::instance(), &App::quit);
  fileMenu->addAction(exitAction);

  // Text Menu (Edit menu adds Start Dectation and Special Characters menus automatically in Mac)
  const QString& textMenuStr = Config::singleton().enableMnemonic() ? tr("&Text") : tr("Text");
  auto editMenu = new PackageMenu(textMenuStr);
  addMenu(editMenu);
  editMenu->setObjectName("edit");
  // we need at least one sub menu to show the Text menu correctly because of this bug.
  // https://bugreports.qt.io/browse/QTBUG-44412?jql=text%20~%20%22qmenubar%20mac%22
  const QString& undoMenuStr = Config::singleton().enableMnemonic() ? tr("&Undo") : tr("Undo");
  editMenu->addAction(new CommandAction("undo", undoMenuStr, "undo"));

  // Find Menu
  const QString& findMenuStr = Config::singleton().enableMnemonic() ? tr("F&ind") : tr("Find");
  auto findMenu = new PackageMenu(findMenuStr);
  addMenu(findMenu);
  findMenu->setObjectName("find");
  const QString& findAndReplaceStr = tr("Find/Replace");
  findMenu->addAction(new CommandAction("find_and_replace", findAndReplaceStr, "find_and_replace"));

  // View menu
  const QString& viewMenuStr = Config::singleton().enableMnemonic() ? tr("&View") : tr("View");
  auto viewMenu = new PackageMenu(viewMenuStr);
  addMenu(viewMenu);
  viewMenu->setObjectName("view");
  const QString& themeMenuStr = Config::singleton().enableMnemonic() ? tr("&Theme") : tr("Theme");
  ThemeMenu* themeMenu = new ThemeMenu(themeMenuStr);
  themeMenu->setObjectName("theme");
  viewMenu->addMenu(themeMenu);
  QActionGroup* themeActionGroup = new QActionGroup(themeMenu);
  for (const QString& name : ThemeManager::sortedThemeNames()) {
    ThemeAction* themeAction = new ThemeAction(name, themeMenu);
    themeMenu->addAction(themeAction);
    themeActionGroup->addAction(themeAction);
  }

  connect(themeActionGroup, &QActionGroup::triggered, this, &MenuBar::themeActionTriggered);

  // Packages menu
  const QString& packageMenuStr =
      Config::singleton().enableMnemonic() ? tr("&Packages") : tr("Packages");
  auto packagesMenu = new PackageMenu(packageMenuStr);
  addMenu(packagesMenu);
  packagesMenu->setObjectName("packages");
  auto packageDevMenu = new PackageMenu(tr("Package Development"));
  packagesMenu->addMenu(packageDevMenu);
  packageDevMenu->addAction(new CommandAction("new_package", tr("&New Package"), "new_package"));
  packageDevMenu->setObjectName("package_development");

  // Settings menu
  const QString& settingsMenuStr =
      Config::singleton().enableMnemonic() ? tr("&Settings") : tr("Settings");
// Qt doc says the merging functionality is based on string matching the title of a QMenu entry.
// But QMenu can't trigger an action (QMenu::triggered doesn't work).
// So On Mac, we need to create both top level QMenu and child Settings QAction.
#ifdef Q_OS_MAC
  auto settingsMenu = new PackageMenu(settingsMenuStr);
  addMenu(settingsMenu);
  settingsMenu->setObjectName("settings");
  QAction* settingsAction = new QAction(settingsMenuStr, settingsMenu);
  settingsMenu->addAction(settingsAction);
#else
  // On Windows, create a top level Settings menu.
  QAction* settingsAction = addAction(settingsMenuStr);
  settingsAction->setObjectName("settings");
#endif
  settingsAction->setMenuRole(QAction::PreferencesRole);
  connect(settingsAction, &QAction::triggered, this, [] { ConfigDialog::showModeless(); });

  // Help menu
  const QString& helpMenuStr = Config::singleton().enableMnemonic() ? tr("&Help") : tr("Help");
  auto helpMenu = new PackageMenu(helpMenuStr);
  addMenu(helpMenu);
  helpMenu->setObjectName("help");
  const QString& aboutMenuStr = Config::singleton().enableMnemonic() ? tr("&About") : tr("About");
  QAction* aboutAction = new QAction(aboutMenuStr, helpMenu);
  aboutAction->setMenuRole(QAction::AboutRole);
  connect(aboutAction, &QAction::triggered, this, &MenuBar::showAboutDialog);
  helpMenu->addAction(aboutAction);
}

void MenuBar::themeActionTriggered(QAction* action) {
  qDebug("themeSelected: %s", qPrintable(action->text()));
  Theme* theme = ThemeManager::theme(action->text());
  Config::singleton().setTheme(theme);
}

void MenuBar::showAboutDialog() {
  AboutDialog dialog;
  dialog.exec();
}

ThemeAction::ThemeAction(const QString& text, QObject* parent) : QAction(text, parent) {
  setObjectName(text);
  setCheckable(true);
}

ThemeMenu::ThemeMenu(const QString& title, QWidget* parent) : QMenu(title, parent) {
  connect(&Config::singleton(), &Config::themeChanged, this, &ThemeMenu::themeChanged);
}

void ThemeMenu::themeChanged(Theme* theme) {
  if (!theme)
    return;

  QAction* action = findChild<QAction*>(theme->name);
  if (action) {
    action->setChecked(true);
  }
}
