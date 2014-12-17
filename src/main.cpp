#include <QMenuBar>
#include <QAction>

#include "OpenRecentItemService.h"
#include "SilkApp.h"
#include "STabWidget.h"
#include "CommandAction.h"
#include "MainWindow.h"
#include "KeymapService.h"
#include "ConfigService.h"
#include "CommandService.h"
#include "commands/ToggleVimEmulationCommand.h"
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
#include "commands/EvalAsRubyCommand.h"
#include "commands/SplitHorizontallyCommand.h"
#include "commands/SplitVerticallyCommand.h"

int main(int argv, char** args) {
  SilkApp app(argv, args);

  LanguageProvider::loadLanguages();

  ConfigService::singleton().load();

  // add commands
  std::unique_ptr<MoveCursorCommand> moveCursorCmd(new MoveCursorCommand);
  CommandService::singleton().add(std::move(moveCursorCmd));

  std::unique_ptr<DeleteCommand> deleteCmd(new DeleteCommand);
  CommandService::singleton().add(std::move(deleteCmd));

  std::unique_ptr<UndoCommand> undoCmd(new UndoCommand);
  CommandService::singleton().add(std::move(undoCmd));

  std::unique_ptr<RedoCommand> redoCmd(new RedoCommand);
  CommandService::singleton().add(std::move(redoCmd));

  std::unique_ptr<EvalAsRubyCommand> evalAsRubyCmd(new EvalAsRubyCommand);
  CommandService::singleton().add(std::move(evalAsRubyCmd));

  std::unique_ptr<OpenFileCommand> openFileCmd(new OpenFileCommand());
  CommandService::singleton().add(std::move(openFileCmd));

  std::unique_ptr<NewFileCommand> newFileCmd(new NewFileCommand());
  CommandService::singleton().add(std::move(newFileCmd));

  std::unique_ptr<SaveFileCommand> saveFileCmd(new SaveFileCommand());
  CommandService::singleton().add(std::move(saveFileCmd));

  std::unique_ptr<SaveAsCommand> saveAsCmd(new SaveAsCommand());
  CommandService::singleton().add(std::move(saveAsCmd));

  std::unique_ptr<SaveAllCommand> saveAllCmd(new SaveAllCommand());
  CommandService::singleton().add(std::move(saveAllCmd));

  CommandService::singleton().add(std::move(std::unique_ptr<CloseTabCommand>(new CloseTabCommand)));

  CommandService::singleton().add(
      std::move(std::unique_ptr<CloseAllTabsCommand>(new CloseAllTabsCommand)));

  CommandService::singleton().add(
      std::move(std::unique_ptr<CloseOtherTabsCommand>(new CloseOtherTabsCommand)));

  CommandService::singleton().add(
      std::move(std::unique_ptr<ReopenLastClosedFileCommand>(new ReopenLastClosedFileCommand)));

  CommandService::singleton().add(std::move(std::unique_ptr<CutCommand>(new CutCommand)));
  CommandService::singleton().add(std::move(std::unique_ptr<CopyCommand>(new CopyCommand)));
  CommandService::singleton().add(std::move(std::unique_ptr<PasteCommand>(new PasteCommand)));
  CommandService::singleton().add(std::move(std::unique_ptr<SelectAllCommand>(new SelectAllCommand)));

  std::unique_ptr<SplitHorizontallyCommand> splitHorizontallyCmd(new SplitHorizontallyCommand());
  CommandService::singleton().add(std::move(splitHorizontallyCmd));

  std::unique_ptr<SplitVerticallyCommand> splitVerticallyCmd(new SplitVerticallyCommand());
  CommandService::singleton().add(std::move(splitVerticallyCmd));

  ViEngine viEngine;

  std::unique_ptr<ToggleVimEmulationCommand> toggleVimEmulationCmd(
      new ToggleVimEmulationCommand(&viEngine));
  CommandService::singleton().add(std::move(toggleVimEmulationCmd));

  //   Load keymap settings after registering commands
  KeymapService::singleton().load();

  MainWindow* w = MainWindow::create();
  w->activeTabWidget()->addNew();
  w->show();

  //   Set focus to active edit view
  if (auto v = w->activeTabWidget()->activeEditView()) {
    v->setFocus();
  }

  if (ConfigService::singleton().isTrue("enable_vim_emulation")) {
    viEngine.enable();
  }

  QMenuBar menuBar(nullptr);

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
  auto fileMenu = menuBar.addMenu(QObject::tr("&File"));
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
  auto editMenu = menuBar.addMenu(QObject::tr("&Edit"));
  editMenu->addAction(undoAction);
  editMenu->addAction(redoAction);
  editMenu->addSeparator();
  editMenu->addAction(cutAction);
  editMenu->addAction(copyAction);
  editMenu->addAction(pasteAction);
  editMenu->addAction(selectAllAction);

  return app.exec();
}
