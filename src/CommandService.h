#pragma once

#include <memory>
#include <unordered_map>
#include <QString>
#include <QVariant>

#include "macros.h"
#include "ICommand.h"
#include "stlSpecialization.h"

class CommandService {
  DISABLE_COPY_AND_MOVE(CommandService)

 public:
  static void runCommand(const QString& name,
                         const CommandArgument& args = CommandArgument(),
                         int repeat = 1);
  static void add(std::unique_ptr<ICommand> cmd);
  static void remove(const QString& name);
  static void init();

 private:
  CommandService() = delete;
  ~CommandService() = delete;

  // QHash doesn't like unique_ptr (probably lack of move semantics),
  // so use an unordered_map here instead
  static std::unordered_map<QString, std::unique_ptr<ICommand>> m_commands;
};
