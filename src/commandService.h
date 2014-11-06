#pragma once

#include <unordered_map>
#include <QString>
#include <QVariant>

#include "macros.h"
#include "singleton.h"
#include "commands/ICommand.h"
#include "stlSpecialization.h"

class CommandService : public Singleton<CommandService> {
  DISABLE_COPY_AND_MOVE(CommandService)

 public:
  ~CommandService() = default;

  void runCommand(
      const QString& name,
      const std::unordered_map<QString, QVariant>& args = std::unordered_map<QString, QVariant>());
  void addCommand(std::unique_ptr<ICommand> cmd);

 private:
  friend class Singleton<CommandService>;
  CommandService() = default;

  // QHash doesn't like unique_ptr (probably lack of move semantics),
  // so use an unordered_map here instead
  std::unordered_map<QString, std::unique_ptr<ICommand>> m_commands;
};
