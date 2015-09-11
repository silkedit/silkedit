#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <functional>
#include <memory>
#include <unordered_map>
#include <QString>
#include <QVariant>

#include "core/macros.h"
#include "core/ICommand.h"
#include "stlSpecialization.h"

class CommandManager {
  DISABLE_COPY_AND_MOVE(CommandManager)

  typedef std::function<std::tuple<bool, std::string, CommandArgument>(const std::string&,
                                                                       const CommandArgument&)>
      CmdEventHandler;

 public:
  static void runCommand(const QString& name,
                         const CommandArgument& args = CommandArgument(),
                         int repeat = 1);
  static void add(std::unique_ptr<core::ICommand> cmd);
  static void remove(const QString& name);
  static void addEventFilter(CmdEventHandler handler);

 private:
  CommandManager() = delete;
  ~CommandManager() = delete;

  // QHash doesn't like unique_ptr (probably lack of move semantics),
  // so use an unordered_map here instead
  static std::unordered_map<QString, std::unique_ptr<core::ICommand>> s_commands;

  static std::vector<CmdEventHandler> s_cmdEventFilters;
};
