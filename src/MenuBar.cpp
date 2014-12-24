#include <vector>
#include <QAction>

#include "commands/OpenFileCommand.h"
#include "commands/NewFileCommand.h"
#include "commands/SaveFileCommand.h"
#include "commands/SaveAsCommand.h"
#include "commands/SaveAllCommand.h"
#include "commands/CloseTabCommand.h"
#include "commands/CloseAllTabsCommand.h"
#include "commands/CloseOtherTabsCommand.h"
#include "commands/ReopenLastClosedFileCommand.h"
#include "commands/MoveCursorCommand.h"
#include "commands/DeleteCommand.h"
#include "commands/UndoCommand.h"
#include "commands/RedoCommand.h"
#include "commands/CutCommand.h"
#include "commands/CopyCommand.h"
#include "commands/PasteCommand.h"
#include "commands/SelectAllCommand.h"
//#include "commands/EvalAsRubyCommand.h"
#include "commands/SplitHorizontallyCommand.h"
#include "commands/SplitVerticallyCommand.h"
#include "MenuBar.h"
#include "CommandAction.h"
#include "OpenRecentItemService.h"
#include "ThemeProvider.h"
#include "Session.h"

MenuBar::MenuBar(QWidget* parent) : QMenuBar(parent) {
  // File Menu actions
  auto openFileAction = new CommandAction(QObject::tr("&Open..."), OpenFileCommand::name);
  auto newFileAction = new CommandAction(QObject::tr("&New File"), NewFileCommand::name);
  auto saveFileAction = new CommandAction(QObject::tr("&Save"), SaveFileCommand::name);
  auto saveAsAction = new CommandAction(QObject::tr("&Save AS..."), SaveAsCommand::name);
  auto saveAllAction = new CommandAction(QObject::tr("&Save All"), SaveAllCommand::name);
  auto closeTabAction = new CommandAction(QObject::tr("&Close"), CloseTabCommand::name);
  auto closeAllTabsAction =
      new CommandAction(QObject::tr("&Close All Tabs"), CloseAllTabsCommand::name);
  auto closeOtherTabsAction =
      new CommandAction(QObject::tr("&Close Other Tabs"), CloseOtherTabsCommand::name);

  // File Menu
  auto fileMenu = addMenu(QObject::tr("&File"));
  fileMenu->addAction(newFileAction);
  fileMenu->addAction(openFileAction);
  fileMenu->addMenu(OpenRecentItemService::singleton().openRecentMenu());
  fileMenu->addAction(saveFileAction);
  fileMenu->addAction(saveAsAction);
  fileMenu->addAction(saveAllAction);
  fileMenu->addAction(closeTabAction);
  fileMenu->addAction(closeAllTabsAction);
  fileMenu->addAction(closeOtherTabsAction);

  // Edit Menu actions
  auto undoAction = new CommandAction(QObject::tr("&Undo"), UndoCommand::name);
  auto redoAction = new CommandAction(QObject::tr("&Redo"), RedoCommand::name);
  auto cutAction = new CommandAction(QObject::tr("&Cut"), CutCommand::name);
  auto copyAction = new CommandAction(QObject::tr("&Copy"), CopyCommand::name);
  auto pasteAction = new CommandAction(QObject::tr("&Paste"), PasteCommand::name);
  auto selectAllAction = new CommandAction(QObject::tr("&Select All"), SelectAllCommand::name);

  // Edit Menu
  auto editMenu = addMenu(QObject::tr("&Edit"));
  editMenu->addAction(undoAction);
  editMenu->addAction(redoAction);
  editMenu->addSeparator();
  editMenu->addAction(cutAction);
  editMenu->addAction(copyAction);
  editMenu->addAction(pasteAction);
  editMenu->addAction(selectAllAction);

  // View menu
  auto viewMenu = addMenu(QObject::tr("&View"));
  auto themeMenu = viewMenu->addMenu(QObject::tr("&Theme"));
  QActionGroup* themeGroup = new QActionGroup(themeMenu);
  for (const QString& name: ThemeProvider::sortedThemeNames()) {
    ThemeAction* themeAction = new ThemeAction(name, themeMenu);
    themeMenu->addAction(themeAction);
    themeGroup->addAction(themeAction);
  }
}

ThemeAction::ThemeAction(const QString& text, QObject* parent): QAction(text, parent) {
  setCheckable(true);
  connect(this, SIGNAL(triggered()), this, SLOT(themeSelected()));
}

void ThemeAction::themeSelected() {
  qDebug("themeSelected");
  Theme* theme = ThemeProvider::theme(text());
  Session::singleton().setTheme(theme);
}
