#pragma once

#include "ICommand.h"
#include "ViEngine.h"

class ChangeModeCommand : public ICommand {
  DISABLE_COPY_AND_MOVE(ChangeModeCommand)
 public:
  ChangeModeCommand(ViEngine* viEngine);
  ~ChangeModeCommand() = default;

 private:
  void doRun(const std::unordered_map<QString, QVariant>& args) override;

  ViEngine* m_viEngine;
};
