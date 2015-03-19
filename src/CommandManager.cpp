#include <QDebug>

#include "CommandManager.h"
#include "commands/ToggleVimEmulationCommand.h"
#include "commands/ReopenLastClosedFileCommand.h"

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
}
