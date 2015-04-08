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

MenuBar* MenuBar::s_globalMenuBar;

void MenuBar::init()
{
  s_globalMenuBar = new MenuBar();
}

MenuBar::MenuBar(QWidget* parent) : QMenuBar(parent) {
  // File Menu
  auto fileMenu = addMenu(QObject::tr("&File"));
  fileMenu->addMenu(OpenRecentItemManager::singleton().openRecentMenu());

  // Text Menu (Edit menu adds Start Dectation and Special Characters menus automatically in Mac)
  auto editMenu = addMenu(QObject::tr("&Text"));
  // we need at least one sub menu to show the Text menu correctly because of this bug.
  // https://bugreports.qt.io/browse/QTBUG-44412?jql=text%20~%20%22qmenubar%20mac%22
  editMenu->addAction(new CommandAction(QObject::tr("&Undo"), UndoCommand::name));

  // View menu
  auto viewMenu = addMenu(QObject::tr("&View"));
  ThemeMenu* themeMenu = new ThemeMenu(QObject::tr("&Theme"));
  viewMenu->addMenu(themeMenu);
  QActionGroup* themeActionGroup = new QActionGroup(themeMenu);
  for (const QString& name : ThemeProvider::sortedThemeNames()) {
    ThemeAction* themeAction = new ThemeAction(name, themeMenu);
    themeMenu->addAction(themeAction);
    themeActionGroup->addAction(themeAction);
  }

  connect(themeActionGroup, &QActionGroup::triggered, this, &MenuBar::themeActionTriggered);

  // Help menu
  auto helpMenu = addMenu(QObject::tr("&Help"));
  QAction* aboutAction = new QAction(QObject::tr("&About"), helpMenu);
  connect(aboutAction, &QAction::triggered, this, &MenuBar::showAboutDialog);
  helpMenu->addAction(aboutAction);
}

void MenuBar::themeActionTriggered(QAction* action) {
  qDebug("themeSelected: %s", qPrintable(action->text()));
  Theme* theme = ThemeProvider::theme(action->text());
  Session::singleton().setTheme(theme);
}

void MenuBar::showAboutDialog() {
  QMessageBox::about(this, SilkApp::applicationName(), "version " + SilkApp::applicationVersion());
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
