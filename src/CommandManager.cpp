#include <QDebug>

#include "CommandManager.h"
#include "commands/ToggleVimEmulationCommand.h"
#include "commands/OpenCommand.h"
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
#include "commands/OpenFindPanelCommand.h"

std::unordered_map<QString, std::unique_ptr<ICommand>> CommandManager::m_commands;

void CommandManager::runCommand(const QString& name, const CommandArgument& args, int repeat) {
  if (m_commands.find(name) != m_commands.end()) {
    m_commands[name]->run(args, repeat);
  } else {
    qDebug() << "Can't find a command: " << name;
  }
}

void CommandManager::add(std::unique_ptr<ICommand> cmd) {
  m_commands[cmd->name()] = std::move(cmd);
}

void CommandManager::remove(const QString& name) {
  m_commands.erase(name);
}

void CommandManager::init() {
  // add commands
  std::unique_ptr<MoveCursorCommand> moveCursorCmd(new MoveCursorCommand);
  add(std::move(moveCursorCmd));

  std::unique_ptr<DeleteCommand> deleteCmd(new DeleteCommand);
  add(std::move(deleteCmd));

  std::unique_ptr<UndoCommand> undoCmd(new UndoCommand);
  add(std::move(undoCmd));

  std::unique_ptr<RedoCommand> redoCmd(new RedoCommand);
  add(std::move(redoCmd));

  //  std::unique_ptr<EvalAsRubyCommand> evalAsRubyCmd(new EvalAsRubyCommand);
  //  add(std::move(evalAsRubyCmd));

  std::unique_ptr<OpenCommand> openFileCmd(new OpenCommand());
  add(std::move(openFileCmd));

  std::unique_ptr<NewFileCommand> newFileCmd(new NewFileCommand());
  add(std::move(newFileCmd));

  std::unique_ptr<SaveFileCommand> saveFileCmd(new SaveFileCommand());
  add(std::move(saveFileCmd));

  std::unique_ptr<SaveAsCommand> saveAsCmd(new SaveAsCommand());
  add(std::move(saveAsCmd));

  std::unique_ptr<SaveAllCommand> saveAllCmd(new SaveAllCommand());
  add(std::move(saveAllCmd));

  add(std::move(std::unique_ptr<CloseTabCommand>(new CloseTabCommand)));

  add(std::move(std::unique_ptr<CloseAllTabsCommand>(new CloseAllTabsCommand)));

  add(std::move(std::unique_ptr<CloseOtherTabsCommand>(new CloseOtherTabsCommand)));

  add(std::move(std::unique_ptr<ReopenLastClosedFileCommand>(new ReopenLastClosedFileCommand)));

  add(std::move(std::unique_ptr<CutCommand>(new CutCommand)));
  add(std::move(std::unique_ptr<CopyCommand>(new CopyCommand)));
  add(std::move(std::unique_ptr<PasteCommand>(new PasteCommand)));
  add(std::move(std::unique_ptr<SelectAllCommand>(new SelectAllCommand)));

  std::unique_ptr<SplitHorizontallyCommand> splitHorizontallyCmd(new SplitHorizontallyCommand());
  add(std::move(splitHorizontallyCmd));

  std::unique_ptr<SplitVerticallyCommand> splitVerticallyCmd(new SplitVerticallyCommand());
  add(std::move(splitVerticallyCmd));

  add(std::move(std::unique_ptr<OpenFindPanelCommand>(new OpenFindPanelCommand)));
}
