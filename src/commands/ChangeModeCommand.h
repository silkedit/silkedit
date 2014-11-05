#pragma once

#include "command.h"
#include "../viEngine.h"

class ChangeModeCommand : public ICommand {

public:
  ChangeModeCommand(ViEngine *viEngine);

private:
  void doRun(const std::unordered_map<QString, QVariant> &args) override;

  ViEngine *m_viEngine;
};
