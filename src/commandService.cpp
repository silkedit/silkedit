#include <QDebug>

#include "commandService.h"

void CommandService::runCommand(const QString& name,
                                const std::unordered_map<QString, QVariant>& args) {
  if (m_commands.find(name) != m_commands.end()) {
    m_commands[name]->run(args);
  } else {
    qDebug() << "Can't find a command: " << name;
  }
}

void CommandService::addCommand(std::unique_ptr<ICommand> cmd) {
  m_commands[cmd->name()] = std::move(cmd);
}
