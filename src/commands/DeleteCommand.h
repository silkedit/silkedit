#pragma once

#include "ICommand.h"
#include "ViEditView.h"

class DeleteCommand : public ICommand {
 public:
  DeleteCommand(ViEditView* viEditView);
  ~DeleteCommand() = default;
  DEFAULT_COPY_AND_MOVE(DeleteCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;

  ViEditView* m_viEditView;
};
