#pragma once

#include "command.h"
#include "../viEngine.h"

class ChangeModeCommand : public ICommand {

public:
  ChangeModeCommand(ViEngine* viEngine);

private:
  void doRun() override;

  ViEngine* m_viEngine;
};
