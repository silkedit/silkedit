#include <QApplication>

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

  ViEngine viEngine;

  std::unique_ptr<ToggleVimEmulationCommand> toggleVimEmulationCmd(
      new ToggleVimEmulationCommand(&viEngine));
  CommandService::singleton().add(std::move(toggleVimEmulationCmd));

  // Load keymap settings after registering commands
  KeymapService::singleton().load();

  MainWindow* w = MainWindow::create();
  w->show();

  if (ConfigService::singleton().isTrue("enable_vim_emulation")) {
    viEngine.enable();
  }

  return app.exec();
}
