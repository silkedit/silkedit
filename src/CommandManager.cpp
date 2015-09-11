#include <QDebug>

#include "CommandManager.h"
#include "PluginManager.h"

using core::ICommand;

std::unordered_map<QString, std::unique_ptr<ICommand>> CommandManager::s_commands;
std::vector<CommandManager::CmdEventHandler> CommandManager::s_cmdEventFilters;

void CommandManager::runCommand(const QString& name, const CommandArgument& args, int repeat) {
  // Copy command name and argument.
  std::string cmdName = name.toUtf8().constData();
  CommandArgument cmdArg = args;

  for (const CmdEventHandler& handler : s_cmdEventFilters) {
    std::tuple<bool, std::string, CommandArgument> resultTuple = handler(cmdName, cmdArg);
    if (std::get<0>(resultTuple)) {
      return;
    }
    cmdName = std::get<1>(resultTuple);
    cmdArg = std::get<2>(resultTuple);
  }

  QString qCmdName = QString::fromUtf8(cmdName.c_str());
  //  qDebug("qCmdName: %s", qPrintable(qCmdName));
  if (s_commands.find(qCmdName) != s_commands.end()) {
    s_commands[qCmdName]->run(cmdArg, repeat);
    PluginManager::singleton().sendCommandEvent(qCmdName, cmdArg);
  } else {
    qDebug() << "Can't find a command: " << qCmdName;
  }
}

void CommandManager::add(std::unique_ptr<ICommand> cmd) {
  s_commands[cmd->name()] = std::move(cmd);
}

void CommandManager::remove(const QString& name) {
  s_commands.erase(name);
}

void CommandManager::addEventFilter(CommandManager::CmdEventHandler handler) {
  s_cmdEventFilters.push_back(handler);
}
