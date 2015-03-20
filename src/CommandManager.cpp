#include <QDebug>

#include "CommandManager.h"
#include "commands/ToggleVimEmulationCommand.h"
#include "commands/ReopenLastClosedFileCommand.h"
#include "commands/MoveCursorCommand.h"
#include "commands/DeleteCommand.h"
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

  std::unique_ptr<SplitHorizontallyCommand> splitHorizontallyCmd(new SplitHorizontallyCommand());
  add(std::move(splitHorizontallyCmd));

  std::unique_ptr<SplitVerticallyCommand> splitVerticallyCmd(new SplitVerticallyCommand());
  add(std::move(splitVerticallyCmd));

  add(std::move(std::unique_ptr<OpenFindPanelCommand>(new OpenFindPanelCommand)));

  add(std::move(std::unique_ptr<ReopenLastClosedFileCommand>(new ReopenLastClosedFileCommand)));
}
