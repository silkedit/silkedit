#pragma once

#include <QString>

#include "macros.h"
#include "core/ICommand.h"

class PluginCommand : public core::ICommand {
 public:
  PluginCommand(const QString& name);
  ~PluginCommand() = default;
  DEFAULT_COPY_AND_MOVE(PluginCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
