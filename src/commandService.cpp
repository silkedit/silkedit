#include <QDebug>

#include "commandService.h"

void CommandService::runCommand(const QString &name)
{
  if (m_commands.find(name) != m_commands.end()) {
    m_commands[name]->run();
  } else {
    qDebug() << "Can't find a command: " << name;
  }
}

void CommandService::addCommand(const QString &name, std::unique_ptr<ICommand> cmd)
{
  m_commands[name] = std::move(cmd);
}
