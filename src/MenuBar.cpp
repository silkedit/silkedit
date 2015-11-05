#include <vector>
#include <QAction>
#include <QMessageBox>

#include "commands/ReopenLastClosedFileCommand.h"
#include "MenuBar.h"
#include "CommandAction.h"
#include "OpenRecentItemManager.h"
#include "core/ThemeProvider.h"
#include "core/Config.h"
#include "SilkApp.h"
#include "commands/PluginCommand.h"
#include "CommandManager.h"
#include "version.h"
#include "ConfigDialog.h"

using core::ThemeProvider;
using core::Theme;
using core::Config;

MenuBar* MenuBar::s_globalMenuBar;

void MenuBar::init() {
  s_globalMenuBar = new MenuBar();
}

MenuBar::MenuBar(QWidget* parent) : QMenuBar(parent) {
  // File Menu
  const QString& fileMenuStr = Config::singleton().enableMnemonic() ? tr("&File") : tr("File");
  auto fileMenu = addMenu(fileMenuStr);
  fileMenu->addMenu(OpenRecentItemManager::singleton().openRecentMenu());
  fileMenu->setObjectName("file");

  // Text Menu (Edit menu adds Start Dectation and Special Characters menus automatically in Mac)
  const QString& textMenuStr = Config::singleton().enableMnemonic() ? tr("&Text") : tr("Text");
  auto editMenu = addMenu(textMenuStr);
  editMenu->setObjectName("edit");
  // we need at least one sub menu to show the Text menu correctly because of this bug.
  // https://bugreports.qt.io/browse/QTBUG-44412?jql=text%20~%20%22qmenubar%20mac%22
  const QString& undoMenuStr = Config::singleton().enableMnemonic() ? tr("&Undo") : tr("Undo");
  editMenu->addAction(new CommandAction("undo", undoMenuStr, "undo"));

  // View menu
  const QString& viewMenuStr = Config::singleton().enableMnemonic() ? tr("&View") : tr("View");
  auto viewMenu = addMenu(viewMenuStr);
  viewMenu->setObjectName("view");
  const QString& themeMenuStr = Config::singleton().enableMnemonic() ? tr("&Theme") : tr("Theme");
  ThemeMenu* themeMenu = new ThemeMenu(themeMenuStr);
  themeMenu->setObjectName("theme");
  viewMenu->addMenu(themeMenu);
  QActionGroup* themeActionGroup = new QActionGroup(themeMenu);
  for (const QString& name : ThemeProvider::sortedThemeNames()) {
    ThemeAction* themeAction = new ThemeAction(name, themeMenu);
    themeMenu->addAction(themeAction);
    themeActionGroup->addAction(themeAction);
  }

  connect(themeActionGroup, &QActionGroup::triggered, this, &MenuBar::themeActionTriggered);

  // Packages menu
  const QString& packageMenuStr =
      Config::singleton().enableMnemonic() ? tr("&Packages") : tr("Packages");
  auto packagesMenu = addMenu(packageMenuStr);
  packagesMenu->setObjectName("packages");
  auto bundleDevelopmentMenu = packagesMenu->addMenu(tr("Package Development"));
  bundleDevelopmentMenu->addAction(
      new CommandAction("new_package", tr("&New Package"), "new_package"));
  bundleDevelopmentMenu->setObjectName("package_development");

  // Settings menu
  const QString& settingsMenuStr =
      Config::singleton().enableMnemonic() ? tr("&Settings") : tr("Settings");
// Qt doc says the merging functionality is based on string matching the title of a QMenu entry.
// But QMenu can't trigger an action (QMenu::triggered doesn't work).
// So On Mac, we need to create both top level QMenu and child Settings QAction.
#ifdef Q_OS_MAC
  auto settingsMenu = addMenu(settingsMenuStr);
  settingsMenu->setObjectName("settings");
  QAction* settingsAction = new QAction(settingsMenuStr, settingsMenu);
  settingsMenu->addAction(settingsAction);
#else
  // On Windows, create a top level Settings menu.
  QAction* settingsAction = addAction(settingsMenuStr);
  settingsAction->setObjectName("settings");
#endif
  settingsAction->setMenuRole(QAction::PreferencesRole);
  connect(settingsAction, &QAction::triggered, this, &MenuBar::showConfigDialog);

  // Help menu
  const QString& helpMenuStr = Config::singleton().enableMnemonic() ? tr("&Help") : tr("Help");
  auto helpMenu = addMenu(helpMenuStr);
  helpMenu->setObjectName("help");
  const QString& aboutMenuStr = Config::singleton().enableMnemonic() ? tr("&About") : tr("About");
  QAction* aboutAction = new QAction(aboutMenuStr, helpMenu);
  aboutAction->setMenuRole(QAction::AboutRole);
  connect(aboutAction, &QAction::triggered, this, &MenuBar::showAboutDialog);
  helpMenu->addAction(aboutAction);
}

void MenuBar::themeActionTriggered(QAction* action) {
  qDebug("themeSelected: %s", qPrintable(action->text()));
  Theme* theme = ThemeProvider::theme(action->text());
  Config::singleton().setTheme(theme);
}

void MenuBar::showAboutDialog() {
  QMessageBox::about(this, SilkApp::applicationName(), tr("version") + " " +
                                                           SilkApp::applicationVersion() + " (" +
                                                           tr("build") + ": " + BUILD + ")");
}

void MenuBar::showConfigDialog() {
  ConfigDialog dialog(this);
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
