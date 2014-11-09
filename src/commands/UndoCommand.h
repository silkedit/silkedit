#pragma once

#include "ICommand.h"
#include "ViEditView.h"

class UndoCommand : public ICommand {
 public:
  UndoCommand(ViEditView* viEditView);
  ~UndoCommand() = default;
  DEFAULT_COPY_AND_MOVE(UndoCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;

  ViEditView* m_viEditView;
};
