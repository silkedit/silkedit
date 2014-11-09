#pragma once

#include "ICommand.h"
#include "ViEngine.h"

class ChangeModeCommand : public ICommand {
 public:
  ChangeModeCommand(ViEngine* viEngine);
  ~ChangeModeCommand() = default;
  DEFAULT_COPY_AND_MOVE(ChangeModeCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;

  ViEngine* m_viEngine;
};
