#pragma once

#include <v8.h>
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
#include "core/FunctionInfo.h"

class CommandManager : public QObject, public core::Singleton<CommandManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(CommandManager)

  typedef std::function<std::tuple<bool, std::string, CommandArgument>(const std::string&,
                                                                       const CommandArgument&)>
      CmdEventHandler;

 public:
  ~CommandManager() = default;

  QString cmdDescription(const QString& name);
  void runCommand(QString cmdName,
                  CommandArgument cmdArgs = CommandArgument(),
                  int repeat = 1);
  void add(std::unique_ptr<ICommand> cmd);
  const std::unordered_map<QString, std::unique_ptr<ICommand>>& commands() { return m_commands; }

public slots:
  void add(const QString& name, const QString& description);
  void remove(const QString& name);

  // internal (only used in initialization in JS side)
  void _assignJSCommandEventFilter(core::FunctionInfo info);

 signals:
  void commandRemoved(const QString& name);

 private:
  friend class core::Singleton<CommandManager>;
  CommandManager() = default;

  // QHash doesn't like unique_ptr (probably lack of move semantics),
  // so use an unordered_map here instead
  std::unordered_map<QString, std::unique_ptr<ICommand>> m_commands;

  v8::UniquePersistent<v8::Function> m_jsCmdEventFilter;

  bool runCommandEventFilter(QString &cmdName, CommandArgument &cmdArg);
};
