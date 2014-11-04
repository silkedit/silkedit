#pragma once

#include "command.h"
#include "../viEngine.h"

class ChangeToInsertModeCommand : public ICommand {

public:
  ChangeToInsertModeCommand(ViEngine* viEngine);

private:
  void doRun() override;

  ViEngine* m_viEngine;
};
