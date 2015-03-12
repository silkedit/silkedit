#include <vector>
#include <QAction>

#include "commands/ReopenLastClosedFileCommand.h"
#include "commands/MoveCursorCommand.h"
#include "commands/SplitHorizontallyCommand.h"
#include "commands/SplitVerticallyCommand.h"
#include "MenuBar.h"
#include "CommandAction.h"
#include "OpenRecentItemManager.h"
#include "ThemeProvider.h"
#include "Session.h"

MenuBar::MenuBar(QWidget* parent) : QMenuBar(parent) {
  // File Menu
  auto fileMenu = addMenu(QObject::tr("&File"));
  fileMenu->addMenu(OpenRecentItemManager::singleton().openRecentMenu());

  // Edit Menu (Without this, Edit menu doesn't appear in Mac)
  auto editMenu = addMenu(QObject::tr("&Edit"));

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

  connect(
      themeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(themeActionTriggered(QAction*)));
}

void MenuBar::themeActionTriggered(QAction* action) {
  qDebug("themeSelected: %s", qPrintable(action->text()));
  Theme* theme = ThemeProvider::theme(action->text());
  Session::singleton().setTheme(theme);
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
