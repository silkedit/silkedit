#pragma once

#include <QString>

#include "core/macros.h"
#include "ICommand.h"

class PluginCommand : public ICommand {
 public:
  PluginCommand(const QString& name, const QString& description);
  ~PluginCommand() = default;
  DEFAULT_COPY_AND_MOVE(PluginCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
