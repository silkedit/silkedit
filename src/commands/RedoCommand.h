#pragma once

#include "ICommand.h"
#include "ViEditView.h"

class RedoCommand : public ICommand {
 public:
  RedoCommand(ViEditView* viEditView);
  ~RedoCommand() = default;
  DEFAULT_COPY_AND_MOVE(RedoCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;

  ViEditView* m_viEditView;
};
