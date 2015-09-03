#include <vector>
#include <QAction>
#include <QMessageBox>

#include "commands/ReopenLastClosedFileCommand.h"
#include "commands/UndoCommand.h"
#include "MenuBar.h"
#include "CommandAction.h"
#include "OpenRecentItemManager.h"
#include "ThemeProvider.h"
#include "Session.h"
#include "SilkApp.h"
#include "commands/PluginCommand.h"
#include "CommandManager.h"
#include "ConfigManager.h"
#include "version.h"

MenuBar* MenuBar::s_globalMenuBar;

void MenuBar::init() {
  s_globalMenuBar = new MenuBar();
}

MenuBar::MenuBar(QWidget* parent) : QMenuBar(parent) {
  // File Menu
  const QString& fileMenuStr = ConfigManager::enableMnemonic() ? tr("&File") : tr("File");
  auto fileMenu = addMenu(fileMenuStr);
  fileMenu->addMenu(OpenRecentItemManager::singleton().openRecentMenu());

  // Text Menu (Edit menu adds Start Dectation and Special Characters menus automatically in Mac)
  const QString& textMenuStr = ConfigManager::enableMnemonic() ? tr("&Text") : tr("Text");
  auto editMenu = addMenu(textMenuStr);
  // we need at least one sub menu to show the Text menu correctly because of this bug.
  // https://bugreports.qt.io/browse/QTBUG-44412?jql=text%20~%20%22qmenubar%20mac%22
  const QString& undoMenuStr = ConfigManager::enableMnemonic() ? tr("&Undo") : tr("Undo");
  editMenu->addAction(new CommandAction(undoMenuStr, UndoCommand::name));

  // View menu
  const QString& viewMenuStr = ConfigManager::enableMnemonic() ? tr("&View") : tr("View");
  auto viewMenu = addMenu(viewMenuStr);
  const QString& themeMenuStr = ConfigManager::enableMnemonic() ? tr("&Theme") : tr("Theme");
  ThemeMenu* themeMenu = new ThemeMenu(themeMenuStr);
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
      ConfigManager::enableMnemonic() ? tr("&Packages") : tr("Packages");
  auto packagesMenu = addMenu(packageMenuStr);
  auto bundleDevelopmentMenu = packagesMenu->addMenu("Package Development");
  bundleDevelopmentMenu->addAction(new CommandAction("New Package", "new_package"));

  // Help menu
  const QString& helpMenuStr = ConfigManager::enableMnemonic() ? tr("&Help") : tr("Help");
  auto helpMenu = addMenu(helpMenuStr);
  const QString& abountMenuStr = ConfigManager::enableMnemonic() ? tr("&About") : tr("About");
  QAction* aboutAction = new QAction(abountMenuStr, helpMenu);
  connect(aboutAction, &QAction::triggered, this, &MenuBar::showAboutDialog);
  helpMenu->addAction(aboutAction);
}

void MenuBar::themeActionTriggered(QAction* action) {
  qDebug("themeSelected: %s", qPrintable(action->text()));
  Theme* theme = ThemeProvider::theme(action->text());
  Session::singleton().setTheme(theme);
}

void MenuBar::showAboutDialog() {
  QMessageBox::about(this, SilkApp::applicationName(),
                     "version " + SilkApp::applicationVersion() + " (build: " + BUILD + ")");
}

ThemeAction::ThemeAction(const QString& text, QObject* parent) : QAction(text, parent) {
  setObjectName(text);
  setCheckable(true);
}

ThemeMenu::ThemeMenu(const QString& title, QWidget* parent) : QMenu(title, parent) {
  connect(&Session::singleton(), SIGNAL(themeChanged(Theme*)), this, SLOT(themeChanged(Theme*)));
}

void ThemeMenu::themeChanged(Theme* theme) {
  if (!theme)
    return;

  QAction* action = findChild<QAction*>(theme->name);
  if (action) {
    action->setChecked(true);
  }
}
