#pragma once

#include "ICommand.h"

class ViEngine;

class ToggleVimEmulationCommand : public ICommand {
 public:
  static const QString name;

  ToggleVimEmulationCommand(ViEngine* viEngine);
  ~ToggleVimEmulationCommand() = default;
  DEFAULT_COPY_AND_MOVE(ToggleVimEmulationCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;

  ViEngine* m_viEngine;
};
