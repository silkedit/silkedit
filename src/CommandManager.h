#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <functional>
#include <memory>
#include <unordered_map>
#include <QString>
#include <QVariant>

#include "ICommand.h"
#include "core/macros.h"
#include "core/stlSpecialization.h"
#include "core/Singleton.h"

class CommandManager : public QObject, public core::Singleton<CommandManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(CommandManager)

  typedef std::function<std::tuple<bool, std::string, CommandArgument>(const std::string&,
                                                                       const CommandArgument&)>
      CmdEventHandler;

 public:
  ~CommandManager() = default;

  QString cmdDescription(const QString& name);
  void runCommand(const QString& name,
                  const CommandArgument& args = CommandArgument(),
                  int repeat = 1);
  void add(std::unique_ptr<ICommand> cmd);
  void remove(const QString& name);
  void addEventFilter(CmdEventHandler handler);

 signals:
  void commandRemoved(const QString& name);

 private:
  friend class Singleton<CommandManager>;
  CommandManager() = default;

  // QHash doesn't like unique_ptr (probably lack of move semantics),
  // so use an unordered_map here instead
  std::unordered_map<QString, std::unique_ptr<ICommand>> m_commands;

  std::vector<CmdEventHandler> m_cmdEventFilters;
};
