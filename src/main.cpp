#include <QApplication>
#include <QMenuBar>
#include <QAction>

#include "STabWidget.h"
#include "CommandAction.h"
#include "MainWindow.h"
#include "KeymapService.h"
#include "ConfigService.h"
#include "CommandService.h"
#include "commands/ToggleVimEmulationCommand.h"
#include "commands/OpenFileCommand.h"
#include "commands/MoveCursorCommand.h"
#include "commands/DeleteCommand.h"
#include "commands/UndoCommand.h"
#include "commands/RedoCommand.h"
#include "commands/EvalAsRubyCommand.h"
#include "commands/SplitHorizontallyCommand.h"

int main(int argv, char** args) {
  QApplication app(argv, args);

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

  std::unique_ptr<SplitHorizontallyCommand> splitHorizontallyCmd(new SplitHorizontallyCommand());
  CommandService::singleton().add(std::move(splitHorizontallyCmd));

  ViEngine viEngine;

  std::unique_ptr<ToggleVimEmulationCommand> toggleVimEmulationCmd(
      new ToggleVimEmulationCommand(&viEngine));
  CommandService::singleton().add(std::move(toggleVimEmulationCmd));

  // Load keymap settings after registering commands
  KeymapService::singleton().load();

  MainWindow* w = MainWindow::create();
  w->show();
  w->activeTabWidget()->addNew();

  // Set focus to active edit view
  if (auto v = w->activeTabWidget()->activeEditView()) {
    v->setFocus();
  }

  if (ConfigService::singleton().isTrue("enable_vim_emulation")) {
    viEngine.enable();
  }

  QMenuBar menuBar(nullptr);
  auto openFileAction = new CommandAction(QObject::tr("&Open..."), OpenFileCommand::name);

  auto fileMenu = menuBar.addMenu(QObject::tr("&File"));
  fileMenu->addAction(openFileAction);

  return app.exec();
}
